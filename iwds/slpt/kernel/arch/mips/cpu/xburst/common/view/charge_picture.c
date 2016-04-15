#include <config.h>
#include <common.h>
#include <malloc.h>
#include <linux/err.h>
#include <command.h>
#include <slpt_app.h>
#include <slpt.h>

#include <slpt_clock.h>
#include <charge_picture.h>

struct charge_pic global_charge_pic;
struct charge_pic global_chargefull_pic;

int slpt_init_charge_picture(void) {
	struct slpt_app_res *clock_dir;

	clock_dir = slpt_kernel_name_to_app_res("clock", uboot_slpt_task);
	assert(clock_dir);

	if (init_flash_pic_view(&global_charge_pic.fpicv, "charge_pic", "clock/charge_pic")) {
		slpt_kernel_printf("charge_pic: failed to init charge pic view\n");
		return -ENODEV;
	}

	if (!slpt_register_view(&global_charge_pic.fpicv.view, clock_dir, NULL, 0)) {
		slpt_kernel_printf("charge_pic: failed to register charge picture to clock\n");
		return -EINVAL;
	}

	return 0;
}

int slpt_init_chargefull_picture(void) {
	struct slpt_app_res *clock_dir;

	clock_dir = slpt_kernel_name_to_app_res("clock", uboot_slpt_task);
	assert(clock_dir);

	if (init_flash_pic_view(&global_chargefull_pic.fpicv, "chargefull_pic", "clock/chargefull_pic")) {
		slpt_kernel_printf("chargefull_pic: failed to init chargefull pic view\n");
		return -ENODEV;
	}

	if (!slpt_register_view(&global_chargefull_pic.fpicv.view, clock_dir, NULL, 0)) {
		slpt_kernel_printf("chargefull_pic: failed to register chargefull picture to clock\n");
		return -EINVAL;
	}

	return 0;
}
