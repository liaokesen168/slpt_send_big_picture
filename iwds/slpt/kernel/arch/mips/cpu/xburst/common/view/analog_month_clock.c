/*
 * analog_month_clock.c
 *
 *  Created on: May 19, 2015
 *      Author: xblin
 */

#include <config.h>
#include <common.h>
#include <malloc.h>
#include <linux/err.h>
#include <command.h>
#include <slpt_app.h>
#include <slpt.h>

#include <slpt_clock.h>

static struct analog_month_clock clocka;

static void init_default_setting(void) {

}

int slpt_init_analog_month_clock(void) {
	struct slpt_app_res *clock_dir;

	clock_dir = slpt_kernel_name_to_app_res("clock", uboot_slpt_task);
	assert(clock_dir);

	assert(!init_analog_month_clock(&clocka, "analog-month-clock-viewa"));
	assert(slpt_register_analog_month_clock(&clocka, clock_dir));

	init_default_setting();
	slpt_display_add_view(&clocka.clock.view);

	return 0;
}
