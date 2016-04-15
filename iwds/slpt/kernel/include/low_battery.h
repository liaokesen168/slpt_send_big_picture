#ifndef _LOW_BATTERY_H_
#define _LOW_BATTERY_H_

#include <view.h>
#include <slpt.h>

struct low_battery {
	struct flash_pic_view fpv;
	unsigned int low_voltage;
	unsigned int warn_voltage;
	unsigned int level;
	unsigned int cnt;
};

extern struct low_battery low_battery;

static inline unsigned int slpt_low_battery_voltage(void) {
	return low_battery.low_voltage;
};

static inline unsigned int slpt_low_battery_warn_voltage(void) {
	return low_battery.warn_voltage;
};

static inline void display_low_battery(void) {
	unsigned int on;

	if (low_battery.level == SLPTARG_MODE_LOWPOWER) {
		if (!flash_pic_view_is_flash(&low_battery.fpv.view))
			on = 1;
		else
			on = low_battery.cnt++ % 2;
	} else {
		on = 0;
	}

	flash_pic_view_set_display(&low_battery.fpv.view, on);

	view_display(&low_battery.fpv.view);
}

static inline void low_battery_set_level(unsigned int level) {
	low_battery.level = level;
}

static inline int sync_low_battery(void) {
	low_battery.low_voltage = slpt_kernel_get_battery_low_voltage();
	low_battery.warn_voltage = slpt_kernel_get_low_battery_warn_voltage();
	return view_sync_setting(&low_battery.fpv.view);
}

extern int init_low_battery(void);

#endif /* _LOW_BATTERY_H_ */
