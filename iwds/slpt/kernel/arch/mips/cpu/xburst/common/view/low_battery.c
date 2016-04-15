#include <config.h>
#include <asm/errno.h>
#include <common.h>
#include <malloc.h>
#include <linux/pr_info.h>
#include <view.h>
#include <low_battery.h>

struct low_battery low_battery;

int init_low_battery(void) {
	struct slpt_app_res *clock_dir;

	clock_dir = slpt_kernel_name_to_app_res("clock", uboot_slpt_task);
	assert(clock_dir);

	if (init_flash_pic_view(&low_battery.fpv, "low-battery", "clock/low-battery-low")) {
		slpt_kernel_printf("low battery: failed to init flash pic view\n");
		return -ENODEV;
	}

	if (!slpt_register_view(&low_battery.fpv.view, clock_dir, NULL, 0)) {
		slpt_kernel_printf("global bg: failed to register background to clock\n");
		return -EINVAL;
	}

	low_battery.low_voltage = slpt_kernel_get_battery_low_voltage();
	low_battery.warn_voltage = slpt_kernel_get_low_battery_warn_voltage();

	return 0;
}
