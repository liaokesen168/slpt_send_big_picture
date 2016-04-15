#include <config.h>
#include <common.h>
#include <malloc.h>
#include <linux/err.h>
#include <command.h>
#include <slpt_app.h>
#include <slpt.h>

#include <slpt_clock.h>

static struct date_cn_view datev;

static void init_default_setting(void) {

}

int slpt_init_date_cn_view(void) {

	struct slpt_app_res *clock_dir;

	clock_dir = slpt_kernel_name_to_app_res("clock", uboot_slpt_task);

	assert(clock_dir);

	assert(!init_date_cn_view(&datev, "date-cn-view"));

	assert(slpt_register_date_cn(&datev, clock_dir));

	init_default_setting();
	slpt_display_add_view(&datev.text.view);

	return 0;
}
