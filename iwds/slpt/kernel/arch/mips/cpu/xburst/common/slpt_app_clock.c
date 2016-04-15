/*
 * Copyright (C) 2014 Ingenic Semiconductor Co., Ltd.
 * Authors: Kage Shen <kkshen@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the License, or (at your
 * option) any later version.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>
#include <common.h>
#include <malloc.h>

#include <linux/err.h>

#include <command.h>
#include <slpt_app.h>
#include <slpt.h>

#include "slpt_app_alarm.h"
#include "slpt_app_timer.h"

#include <rtc.h>
#include <asm/arch/jz47xx_rtc.h>

#include <fb_struct.h>

#include <power/jz4780_battery.h>
#include <asm/arch/adc.h>

#include <slpt_clock.h>

#include <linux/pr_info.h>

struct app_clock_params {
	struct timer_task *task;
	unsigned int clock_enable;
	unsigned int clock_period;
	unsigned int clock_count;
	unsigned int clock_sync_time;
	int clock_zone;
	unsigned int clock_choose;
	char clock_type[30];
	unsigned int need_to_reinit;
	unsigned int need_to_display;
	unsigned int need_to_update_task;
	struct {
		int sync;
		unsigned int time;
	} sync_kernel_time;

	int buffer_index;
	unsigned int current_time;
};

#define RTC_SYNC_TIME (10 * 60)

static struct app_clock_params clock_params = {
	.clock_zone = 8 * 60 *60,
	.clock_period  = 1,
	.clock_type = "auto",	/* "auto", "bitmask", "digital", "analog" ""*/
	.clock_choose = CLOCK_TYPE_NONE,
	.sync_kernel_time.sync = 0,
	.sync_kernel_time.time = 0,
};

struct slpt_app_res clock_config_res[] = {
	SLPT_RES_INT_DEF("clock-period", clock_params.clock_period),
	SLPT_RES_INT_DEF("clock-zone", clock_params.clock_zone),
	SLPT_RES_ARR_DEF("clock-type", clock_params.clock_type),
	SLPT_RES_INT_DEF("clock-sync-time-enable", clock_params.sync_kernel_time.sync),
	SLPT_RES_INT_DEF("clock-sync-time", clock_params.sync_kernel_time.time),
};
SLPT_REGISTER_RES_ARR(clock_config, clock_config_res);

#ifdef CONFIG_SLPT_DEBUG
#define OFFSET_X 0
#define OFFSET_Y 0
static unsigned int test_times = 0;
static unsigned int resume_times = 0;

static inline void slpt_test_func(void)
{
	draw_nums(0, 64, test_times ++);
	draw_nums(0, 64 + 32 * 2, resume_times);
}
#else
 #define OFFSET_X 10
 #define OFFSET_Y 64
 #define slpt_test_func()
#endif

static void draw_num_clock(unsigned int time, int reinit)
{
	static unsigned int flag = 0;
	static struct rtc_time otm = {
			.tm_hour = 24,
			.tm_min = 59,
			.tm_sec = 1
	};

	struct rtc_time ntm;

	rtc_time_to_tm(time, &ntm);

	if (reinit)
		lcd_draw_clear();

	if (otm.tm_hour != ntm.tm_hour || reinit) {
		draw_num_64_32(0 + OFFSET_X, 0 + OFFSET_Y, ntm.tm_hour/10);
		draw_num_64_32(32 + OFFSET_X, 0 + OFFSET_Y, ntm.tm_hour%10);
	}

	flag = ~flag;
	draw_maoh_64_32(64 + OFFSET_X, 0 + OFFSET_Y, flag);

	if (otm.tm_min != ntm.tm_min  || reinit) {
		draw_num_64_32(96 + OFFSET_X, 0 + OFFSET_Y, ntm.tm_min/10);
		draw_num_64_32(128 + OFFSET_X, 0 + OFFSET_Y,  ntm.tm_min%10);
	}

	draw_nums(170 + OFFSET_X, 32 + OFFSET_Y, ntm.tm_sec/10);
	draw_nums(186 + OFFSET_X, 32 + OFFSET_Y, ntm.tm_sec%10);

	debug("TDATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
			ntm.tm_year, ntm.tm_mon, ntm.tm_mday, ntm.tm_wday,
			ntm.tm_hour, ntm.tm_min, ntm.tm_sec );

	slpt_test_func();

	memcpy(&otm, &ntm, sizeof(struct rtc_time));
}

extern void _machine_restart(void);

static unsigned long last_slpt_time_sec;
static unsigned long last_slpt_time_nsec;

void set_last_slpt_time(unsigned long tv_sec, unsigned long tv_usec) {
	last_slpt_time_sec = tv_sec;
	last_slpt_time_nsec = tv_usec * 1000;
}

static inline long clock_sync_kernel_time(long tv_sec, long tv_nsec) {
	unsigned long suspend_sys_sec = tv_sec;
	unsigned long suspend_sys_nsec = tv_nsec;
	long final_sec;
	long delay;
	int second_dif;

	second_dif = suspend_sys_sec - last_slpt_time_sec;
	switch(second_dif) {
	case 0:
		delay = (1000000000 - suspend_sys_nsec) / 100000000 * 7;
		mdelay(delay);
		final_sec = suspend_sys_sec + 1;
		break;

	case 1:
		if (last_slpt_time_nsec > 950000000) {
			delay = (1000000000 - suspend_sys_nsec) / 100000000 * 7;
			mdelay(delay);
			final_sec = suspend_sys_sec + 1;
		} else {
			final_sec = suspend_sys_sec;
		}
		break;

	default:
		delay = (1000000000 - suspend_sys_nsec) / 100000000 * 7;
		mdelay(delay);
		final_sec = suspend_sys_sec + 1;
		break;
	}

	return final_sec;
}

int current_clock_type(void) {
	return clock_params.clock_choose;
}

static inline int choose_clock(void)
{
	if (!strncmp(clock_params.clock_type, "bitmask", 7)) {
		return CLOCK_TYPE_BITMASK;
	}
	return CLOCK_TYPE_ANALOG_MIX_DIGITAL;
}

void request_clock_display(void) {
	clock_params.need_to_display = 1;
}

extern void print_tcu2(void);

static inline void do_display(void) {
	if (clock_params.clock_choose == CLOCK_TYPE_BITMASK)
		draw_num_clock(get_currnet_time(), clock_params.need_to_reinit);
	else
		slpt_display();
}

void clock_display_task(void) {
	struct fb_struct *fbs = get_default_fb();
	struct fb_region region = *get_current_fb_region();

	if (!slpt_kernel_get_fb_on())
		return;

	if (!clock_params.need_to_display)
		return;

	board_lcd_power_on();

	clock_params.clock_choose = choose_clock();

#ifdef CONFIG_FB_DOUBLE_BUFFERING
	if (clock_params.need_to_reinit) {
		if (++clock_params.buffer_index >= fbs->nums)
			clock_params.buffer_index = 0;
		region.base = fbs->base + clock_params.buffer_index * fbs->size;
		set_current_fb_region(&region);
		set_current_time(clock_params.current_time);
		do_display();
		lcd_flush_fb(fbs, clock_params.buffer_index * fbs->yres);
	}

	/* start dma */
	lcd_pan_display(fbs, clock_params.buffer_index * fbs->yres);

	/* switch to next framebuffer */
	if (++clock_params.buffer_index >= fbs->nums)
		clock_params.buffer_index = 0;
	region.base = fbs->base + clock_params.buffer_index * fbs->size;
	set_current_fb_region(&region);

	/* set to next second, and draw it */
	set_current_time(clock_params.current_time + clock_params.clock_period);
	do_display();
#else
	set_current_time(clock_params.current_time);
	do_display();
	/* start dma */
	lcd_pan_display(fbs, 0 * fbs->yres);
#endif

	/* wait for lcd dma ending */
	lcd_wait_display_end(fbs);

	board_lcd_power_off();

	lcd_img_display_set_reinit(0);
	clock_params.need_to_reinit = 0;
	clock_params.need_to_display = 0;
}

void clock_task_handler(struct timer_task *task)
{
	int low_battery_level = get_slpt_run_mode();
	unsigned int time;

	clock_params.need_to_update_task = 1;

	if (!slpt_kernel_get_fb_on())
		return;

	debug("task handler call %s\n", __func__);

	if (low_battery_level == SLPTARG_MODE_WARNING) {
		debug("slpt: battery too low, shutdown\n");
		slpt_mode_exit();
	} else if (slpt_low_power_shutdown && (low_battery_level == SLPTARG_MODE_ACTIVE || slpt_charger_online)) {
		debug("slpt: battery is charging in low power shutdown mode, shutdown\n");
		slpt_mode_shutdown();
	}

	time = clock_params.clock_count + clock_params.clock_sync_time + clock_params.clock_zone;
	debug ("clock sync time: %u --> time:%u count:%d zone:%d\n",
		   clock_params.clock_sync_time, time, clock_params.clock_count, clock_params.clock_zone);
	clock_params.current_time = time;

	request_clock_display();

	clock_params.clock_count += clock_params.clock_period;

	update_timer_task(clock_params.task, clock_params.clock_period);
	clock_params.need_to_update_task = 0;
}

static int slpt_clock_init(void)
{
	struct timer_task *task;

	debug("init call %s\n", __func__);

	task = register_timer_task(clock_task_handler);
	if (IS_ERR(task)) {
		printf("app clock register alarm task error\n");
		return -ENOMEM;
	}

	clock_params.task = task;

	slpt_display_init();

	update_timer_task(task, clock_params.clock_period);

	return 0;
}
SLPT_APP_INIT_ONETIME(slpt_clock_init);

static int slpt_clock_update(void)
{
	long tv_sec, tv_nsec;

	if (!slpt_kernel_get_fb_on())
		return 0;

	clock_params.clock_count = 0;
	clock_params.buffer_index = 0;

	if(!slpt_kernel_get_suspend_time(&tv_sec, &tv_nsec))
		clock_params.clock_sync_time = clock_sync_kernel_time(tv_sec, tv_nsec);
	else
		clock_params.clock_sync_time = jz47xx_rtc_get_realtime();

	debug ("clock sync time: %u\n", clock_params.clock_sync_time);

//	jz4780_sadc_init();
#ifdef CONFIG_SLPT_DEBUG
	resume_times ++;
#endif
	clock_params.need_to_reinit = 1;

	slpt_display_sync();
	request_clock_display();
	clock_params.current_time = clock_params.clock_sync_time + clock_params.clock_zone;
	set_current_time(clock_params.current_time);

	if (clock_params.need_to_update_task)
		update_timer_task(clock_params.task, clock_params.clock_period);

	return 0;
}
SLPT_CORE_INIT_EVERYTIME(slpt_clock_update);
