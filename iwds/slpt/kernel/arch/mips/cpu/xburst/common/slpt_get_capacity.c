#include <slpt.h>
#include <linux/pr_info.h>
#include <linux/string.h>

extern const int jz_cv_charging[101];
extern const int jz_cv_discharging[101];

static const int *jz_charging_cv = NULL; /* get the charging capacity curve ,it has default value */
static const int *jz_discharging_cv = NULL; /*get the discharging capacity curve,it has default value*/

static int get_capacity(unsigned int voltage, const int *jz_cv)
{
	int capacity;
	for(capacity = 0; capacity < 101; capacity++) {
		if(voltage <= jz_cv[capacity])
			return capacity;
	}

	return 100;
}

int slpt_get_capacity(unsigned int voltage, unsigned int is_charging)
{
	int capacity = 0;
	const int *jz_cv;

	if(jz_charging_cv == NULL) {
		jz_charging_cv = slpt_kernel_get_battery_charging_cv(); /*only need init the memory one time*/
		if(!jz_charging_cv) {
			pr_err("get the charging capacity curve fail, the curve is default");
			jz_charging_cv = jz_cv_charging;
		}
	}
	if(jz_discharging_cv == NULL) {
		jz_discharging_cv = slpt_kernel_get_battery_discharging_cv();
		if(!jz_discharging_cv) {
			pr_err("get the discharging capacity curve fail, the curve is default");
			jz_discharging_cv = jz_cv_discharging;
		}
	}

	if(is_charging) {
		jz_cv = jz_charging_cv;
	} else {
		jz_cv = jz_discharging_cv;
	}

	capacity = get_capacity(voltage, jz_cv);

	return capacity;
}
