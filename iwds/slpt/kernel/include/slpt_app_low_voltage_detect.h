#ifndef _LOW_VOLTAGE_NOTIFY_H_
#define _LOW_VOLTAGE_NOTIFY_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <linux/list.h>

struct low_voltage_notify {
	struct list_head link;
	void(*callback)(struct low_voltage_notify *no);
};

extern void register_low_voltage_notify(struct low_voltage_notify *no);
extern void unregister_low_voltage_notify(struct low_voltage_notify *no);
extern void low_voltage_notify(void);

extern int low_pmu_voltage_mode(void);
extern void set_low_pmu_voltage_detect_state(int min_vol, int max_vol);

#ifdef __cplusplus
}
#endif
#endif

