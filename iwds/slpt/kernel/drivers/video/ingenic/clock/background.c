#include <config.h>
#include <asm/errno.h>
#include <common.h>
#include <malloc.h>
#include <linux/pr_info.h>
#include <view.h>
#include <slpt.h>
#include <background.h>

struct bg_pic global_bg;

int init_global_bg_pic(void) {
	struct slpt_app_res *clock_dir;
	struct fb_region *bg;
	struct view *pv;

	bg = get_current_fb_region();
	assert(bg);

	clock_dir = slpt_kernel_name_to_app_res("clock", uboot_slpt_task);
	assert(clock_dir);

	pv = alloc_pic_view("background", "clock/background");
	if (!pv) {
		slpt_kernel_printf("global bg: failed to alloc pic view\n");
		return -ENODEV;
	}

	if (!slpt_register_view(pv, clock_dir, NULL, 0)) {
		slpt_kernel_printf("global bg: failed to register background to clock\n");
		return -EINVAL;
	}

	global_bg.pv = pv;
	view_set_bg(global_bg.pv, bg);

	return 0;
}
