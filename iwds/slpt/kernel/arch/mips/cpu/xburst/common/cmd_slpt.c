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

#include <asm/io.h>

#include <command.h>

#include <slpt.h>
#include <slpt_irq.h>
#include <slpt_app.h>

#include <asm/arch/jz47xx_rtc.h>

#include <fb_struct.h>
#include <slpt_clock.h>

#define bool	_Bool
#define true	1
#define false	0

extern void slpt_rtc_check_alarm(void);
extern void update_wakeup_time(void);

static inline unsigned int slpt_initcall_onetime_start(void)
{
	extern unsigned int __slpt_initcall_onetime_start;
	return (unsigned int) &__slpt_initcall_onetime_start;
}

static inline unsigned long slpt_initcall_onetime_end(void)
{
	extern unsigned int __slpt_initcall_onetime_end;
	return (unsigned int) &__slpt_initcall_onetime_end;
}

static inline unsigned int slpt_exitcall_onetime_start(void)
{
	extern unsigned int __slpt_exitcall_onetime_start;
	return (unsigned int) &__slpt_exitcall_onetime_start;
}

static inline unsigned long slpt_exitcall_onetime_end(void)
{
	extern unsigned int __slpt_exitcall_onetime_end;
	return (unsigned int) &__slpt_exitcall_onetime_end;
}

static inline unsigned int slpt_initcall_everytime_start(void)
{
	extern unsigned int __slpt_initcall_everytime_start;
	return (unsigned int) &__slpt_initcall_everytime_start;
}

static inline unsigned long slpt_initcall_everytime_end(void)
{
	extern unsigned int __slpt_initcall_everytime_end;
	return (unsigned int) &__slpt_initcall_everytime_end;
}

static inline unsigned long slpt_exitcall_everytime_start(void)
{
	extern unsigned int __slpt_exitcall_everytime_start;
	return (unsigned int) &__slpt_exitcall_everytime_start;
}

static inline unsigned long slpt_exitcall_everytime_end(void)
{
	extern unsigned int __slpt_exitcall_everytime_end;
	return (unsigned int) &__slpt_exitcall_everytime_end;
}

static int slpt_initcall(void *head, unsigned int end_addr)
{
	int ret;
	init_fnc_t *init_fnc_ptr;

	init_fnc_ptr = (init_fnc_t *)head;

	for (; *init_fnc_ptr && ((unsigned int) init_fnc_ptr < end_addr);
			++init_fnc_ptr) {
		//slpt_kernel_printf("slpt initcall: %p %pf\n", init_fnc_ptr, *init_fnc_ptr);
		ret = (*init_fnc_ptr)();
		if (ret) {
			slpt_kernel_printf("initcall failed "
				"at call %p, return: %d\n", *init_fnc_ptr, ret);
		}
	}

	return 0;
}

extern void xmdelay(volatile unsigned int ms);

void slpt_initcall_onetime(void)
{
	slpt_initcall((void *)slpt_initcall_onetime_start(),
			slpt_initcall_onetime_end());
}

void slpt_exitcall_onetime(void)
{
	slpt_initcall((void *)slpt_exitcall_onetime_start(),
			slpt_exitcall_onetime_end());
}

void inline slpt_initcall_everytime(void)
{
	slpt_initcall((void *)slpt_initcall_everytime_start(),
			slpt_initcall_everytime_end());
}

int slpt_mode_exit(void)
{
	slpt_initcall((void *)slpt_exitcall_everytime_start(),
			slpt_exitcall_everytime_end());
	slpt_kernel_set_go_kernel(1);
	do_gokernel();
	return 0;
}

int slpt_mode_shutdown(void)
{
	debug("slpt shutdown the machine.\n");
	do_reset(NULL, 0, 0, NULL);
	return 0;
}

int slpt_exception_entry(void)
{
	debug("slpt exception entry\n");
	do_gokernel();
	return 0;
}

static unsigned int save_cpu_freq;

static int slpt_cpu_freq_down(void)
{
	unsigned int tmp = readl(0xb0000000);
	save_cpu_freq = tmp & 0xff;
	writel((tmp & (~0xff)) | (1) | (3 << 4), 0xb0000000);
	return 0;
}
SLPT_APP_INIT_EVERYTIME(slpt_cpu_freq_down);

static int slpt_cpu_freq_up(void)
{
	unsigned int tmp = readl(0xb0000000);
	writel((tmp & (~0xff)) | save_cpu_freq, 0xb0000000);
	return 0;
}
SLPT_APP_EXIT_EVERYTIME(slpt_cpu_freq_up);

static unsigned int slpt_run_mode;

static inline int check_slpt_mode(unsigned int m)
{
	if (m >= SLPTARG_MODE_ACTIVE && m <= SLPTARG_MODE_NO_CAPCITY)
		return 0;
	else
		return -EINVAL;
}

unsigned int get_slpt_run_mode(void)
{
	return slpt_run_mode;
}

int set_slpt_run_mode(unsigned int m)
{
	if (check_slpt_mode(m)) {
		debug("set_slpt_mode: Invalid argument\n");
		return -EINVAL;
	}

	slpt_run_mode = m;
	return 0;
}

static inline void display_task(void) {
	/* update_charger_state(); */
	clock_display_task();
}

static inline void common_tasks(void) {
	display_task();
	update_wakeup_time();
}

#ifdef CONFIG_SLPT_MAP_TO_KSEG2
static int cmd_slpt(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	slpt_kernel_set_go_kernel(0);
	set_slpt_run_mode(SLPTARG_MODE_ACTIVE);

	slpt_irq_check_mask();
	slpt_rtc_check_alarm();

	board_lcd_power_on();
	board_lcd_power_off();

	slpt_initcall_everytime();

//	gpio_direction_output(SLPT_DEBUG_PIN, 0);
//	gpio_set_value(SLPT_DEBUG_PIN, 1);
//	gpio_set_value(SLPT_DEBUG_PIN, 0);

	common_tasks();

	do_gokernel();

	return CMD_RET_SUCCESS;
}

void run_every_time(int need_go_kernel) {
	slpt_printf_kernel_mode = 0;

	slpt_do_irq();
	common_tasks();

	if (need_go_kernel)
		slpt_mode_exit();
	else
		do_gokernel();
}

static	volatile unsigned int a;
void slpt_cache_prefetch(void) {
	volatile unsigned int *addr = (unsigned int *)CONFIG_SYS_SDRAM_BASE;

	unsigned int i;

	slpt_printf_kernel_mode = 0;

	for(i=0; i< CONFIG_MEM_SIZE / 32; i++) {
		a = *(addr + i * 8);
	}

	do_gokernel();
}
#else
static int cmd_slpt(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	set_slpt_run_mode(SLPTARG_MODE_ACTIVE);

	slpt_irq_check_mask();

	slpt_rtc_check_alarm();

	board_lcd_power_on();
	board_lcd_power_off();

	slpt_initcall_everytime();

	debug("entry main loop\n");

//	gpio_direction_output(SLPT_DEBUG_PIN, 0);
//	gpio_set_value(SLPT_DEBUG_PIN, 1);
//	gpio_set_value(SLPT_DEBUG_PIN, 0);

	//display_task();
	common_tasks();

	do_suspend(SLPTARG_LOAD_RESETDLL);

	while(true) {
		slpt_do_irq();
		common_tasks();
		do_suspend(0);
	}

	return CMD_RET_SUCCESS;
}

void run_every_time(int need_go_kernel) {

}

void slpt_cache_prefetch(void) {

}
#endif

U_BOOT_CMD(
	slpt, 6, 1, cmd_slpt,
	"slpt system main loop",
	"nothing\n"
	"nothing"
	);
