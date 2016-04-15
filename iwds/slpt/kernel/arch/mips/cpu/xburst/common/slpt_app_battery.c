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
#include <low_battery.h>

#undef pr_debug
#ifdef CONFIG_SLPT_DEBUG_BATTERY
int pr_debug_battery = 1;
#else
int pr_debug_battery = 0;
#endif

#define pr_debug(x...)							\
	do {										\
		if (pr_debug_battery)					\
			pr_err(x);							\
	} while (0)

struct app_battery_params {
	struct timer_task *task;
	unsigned int vbattery;
	unsigned int period;
	unsigned int voltage;

	/* gpio charger */
	int charger_gpio;
	int charger_level;

	/* low battery */
	unsigned int low_voltage;
	unsigned int warn_voltage;

	/* low battery capacity */
	int warn_capacity;

	/* power state */
	const char *power_state;
};

static struct app_battery_params battery_params;

int slpt_low_power_shutdown = 0;
int slpt_charger_online = 0;
int chargefull_flag = 0; /* if the capacity=100, chargefull_flag = 1 ,else it is 0 */

extern int gpio_get_value(int gpio);

static void update_battery_state(void);

void update_charger_state(void) {
	static int online_state = 0;
	int charger_gpio = battery_params.charger_gpio;
	int charger_level = battery_params.charger_level;

	pr_debug ("charger gpio : %d  level: %d val:%d\n",
		   charger_gpio, charger_level, charger_gpio >= 0 ? gpio_get_value(charger_gpio) : -1);

	if (charger_gpio >= 0 && charger_level >= 0)
		slpt_charger_online = gpio_get_value(charger_gpio) == charger_level;

	if (online_state != slpt_charger_online) {
		online_state = slpt_charger_online;
		update_battery_state();
		request_clock_display();
	}
}

static void update_battery_state(void) {
#ifdef CONFIG_M200 
	unsigned int voltage = 4200;
#else
	unsigned int voltage = get_battery_voltage() / 1000; /* kernel using mV */
#endif
	unsigned int low_battery_voltage = battery_params.low_voltage;
	unsigned int low_battery_warn_voltage = battery_params.warn_voltage;
	const char *power_state = battery_params.power_state;
	int capacity = slpt_get_capacity(voltage, slpt_charger_online); /* get capacity */
	int warn_capacity = battery_params.warn_capacity;

	if(capacity >= 100) /* according to the capacity , we show the charge full picture */
		chargefull_flag = 1;
	else
		chargefull_flag = 0;

	if (!strcmp(power_state, SLPT_V_LOW_POWER_SHUTDOWN))
		slpt_low_power_shutdown = 1;

	battery_params.voltage = voltage;
	slpt_kernel_set_battery_voltage(voltage);

	if (!low_battery_voltage)
		low_battery_voltage = BATTERY_LOW_VOLTAGE; /* it is 10% power in s2121b_16t */

	if (voltage <= low_battery_warn_voltage) {
		/* The battery is in a dangerous state */
		set_slpt_run_mode(SLPTARG_MODE_WARNING);
	} else if (voltage <= low_battery_voltage){
		/* The battery is in a low power state */
		set_slpt_run_mode(SLPTARG_MODE_LOWPOWER);
	} else {
		/* The battery is in active state */
		set_slpt_run_mode(SLPTARG_MODE_ACTIVE);
	}

#ifdef CONFIG_VIEW
	low_battery_set_level(get_slpt_run_mode());
#endif

	pr_debug ("low bat:low:%d warn:%d shutdown:%s voltage: %d level: %d\n",
			  low_battery_voltage, low_battery_warn_voltage, power_state, voltage,
			  _IOC_NR(get_slpt_run_mode()));

	/* if the warn_capacity == 0, slpt won't wake up Automatically on low battery state*/
	if((warn_capacity > 0) && (warn_capacity >= capacity)) {
		slpt_kernel_set_battery_wake_flag(1);
		slpt_mode_exit();
	} else {
		slpt_kernel_set_battery_wake_flag(0);
	}
}

extern unsigned int get_wake_capacity(void);

void battery_task_handler(struct timer_task *task)
{
	debug("task handler call %s\n", __func__);

#ifdef CONFIG_SLPT_DEBUG
	if (current_clock_type() == CLOCK_TYPE_BITMASK)
		draw_nums(0, 64 + 32, battery_params.voltage);
#endif

	update_timer_task(battery_params.task, battery_params.period);

	update_battery_state();
}

static int slpt_battery_init_everytime(void)
{
	battery_params.low_voltage = slpt_kernel_get_battery_low_voltage();
	battery_params.warn_voltage = slpt_kernel_get_low_battery_warn_voltage();
	battery_params.charger_gpio = slpt_kernel_get_charger_gpio();
	battery_params.charger_level = slpt_kernel_get_charger_level();
	battery_params.warn_capacity = slpt_kernel_get_warn_capacity();
	battery_params.power_state = slpt_kernel_get_power_state();

	return 0;
}
SLPT_CORE_INIT_EVERYTIME(slpt_battery_init_everytime);

static int slpt_battery_init(void)
{
	struct timer_task *task;

	debug("init call %s\n", __func__);

	task = register_timer_task(battery_task_handler);
	if (IS_ERR(task)) {
		printf("app clock register alarm task error\n");
		return -ENOMEM;
	}

	battery_params.task = task;
	battery_params.period = 60;
	/* update_timer_task(task, battery_params.period); */

	return 0;
}
SLPT_APP_INIT_ONETIME(slpt_battery_init);

static int cmd_battery_set_voltage(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	unsigned int voltage;

	if (argc <= 2) {
		voltage = 3500;
	} else {
		voltage = simple_strtoul(argv[1], NULL, 10);
	}

	printf("voltage is : %u\n", voltage);
	slpt_kernel_set_battery_voltage(voltage);

	return 0;
}

U_BOOT_CMD(
	battery_set_voltage, 10, 1, cmd_battery_set_voltage,
	"set battery voltage",
	"set battery voltage"
	);
