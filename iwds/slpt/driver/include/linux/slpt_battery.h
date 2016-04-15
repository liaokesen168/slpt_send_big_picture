#ifndef _SLPT_BATTERY_H_
#define _SLPT_BATTERY_H_

struct slpt_battery {
	unsigned int voltage;
	unsigned int low_battery_voltage;
};

/* for kernel driver */
void slpt_set_battery_struct(struct slpt_battery *battery);
void slpt_set_battery_voltage(unsigned int voltage);
unsigned int slpt_get_battery_voltage(void);
void slpt_set_battery_low_voltage(unsigned int voltage);

/* for slpt app */
void slpt_app_set_battery_struct(void *data);
void slpt_app_set_battery_voltage(unsigned int voltage);
int slpt_app_get_battery_low_voltage(void);

#endif /* _SLPT_BATTERY_H_ */
