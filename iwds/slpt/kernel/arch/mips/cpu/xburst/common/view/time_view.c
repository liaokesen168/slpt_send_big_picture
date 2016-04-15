#include <config.h>
#include <common.h>
#include <malloc.h>
#include <linux/err.h>
#include <command.h>
#include <slpt_app.h>
#include <slpt.h>

#include <slpt_clock.h>

static struct time_view timev;

static void init_default_setting(void) {

}

int slpt_init_time_view(void) {

	struct slpt_app_res *clock_dir;

	clock_dir = slpt_kernel_name_to_app_res("clock", uboot_slpt_task);

	assert(clock_dir);

	assert(!init_time_view(&timev, "time-view"));

	assert(slpt_register_time(&timev, clock_dir));

	init_default_setting();

	slpt_display_add_view(&timev.text.view);

	return 0;
}
