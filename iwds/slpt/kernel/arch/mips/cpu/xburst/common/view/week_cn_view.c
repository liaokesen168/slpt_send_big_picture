#include <config.h>
#include <common.h>
#include <malloc.h>
#include <linux/err.h>
#include <command.h>
#include <slpt_app.h>
#include <slpt.h>

#include <slpt_clock.h>

static struct week_cn_view weekv;

static void init_default_setting(void) {

}

int slpt_init_week_cn_view(void) {

	struct slpt_app_res *clock_dir;

	clock_dir = slpt_kernel_name_to_app_res("clock", uboot_slpt_task);

	assert(clock_dir);

	assert(!init_week_cn_view(&weekv, "week-cn-view"));

	assert(slpt_register_week_cn(&weekv, clock_dir));

	init_default_setting();
	slpt_display_add_view(&weekv.text.view);

	return 0;
}
