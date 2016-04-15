#include <config.h>
#include <common.h>
#include <slpt.h>
#include <slpt_app.h>
#include <vsprintf.h>
#include <linux/pr_info.h>
#include <linux/err.h>
#undef pr_debug
#ifdef CONFIG_SLPT_DEBUG_KEY
int pr_debug_key = 1;
#else
int pr_debug_key = 0;
#endif

#define pr_debug(x...)							\
	do {										\
		if (pr_debug_key)					\
			pr_err(x);							\
	} while (0)

struct slpt_key {
	int id;						/* unique id in kernel */
	const char *name;			/* unique name in slpt */
	unsigned long val;		    /* default value in slpt */
	unsigned long *val_addr;
};

#undef KEY
#define KEY(index, n, v) [index] = {.id = -1, .name = n, .val = (unsigned long)(v)}

/* main key list stores in kernel, there we just define a part of key list,
 * and define it's defualt value here, in case of failed to get key from kernel
 */
struct slpt_key slpt_key_list[SLPT_K_NUMS] = {
	KEY(SLPT_K_POWER_STATE, "power-state", SLPT_V_POWER_NORMAL),
	KEY(SLPT_K_CHARGER_GPIO, "charger-gpio", -1),
	KEY(SLPT_K_CHARGER_LEVEL, "charger-level", -1),
	KEY(SLPT_K_LOW_BATTERY_WARN_VOL, "low-bat-warn-vol", BATTERY_WARNING_VOLTAGE),
	KEY(SLPT_K_BATTERY_VOLTAGE, "battery-voltage", 0),
	KEY(SLPT_K_BATTERY_LOW_VOLTAGE, "battery-low-voltage", BATTERY_LOW_VOLTAGE),
	KEY(SLPT_K_BATTERY_CAPACITY, "battery-capacity", 0),
	KEY(SLPT_K_BATTERY_WAKE_FLAG, "battery-wake-flag", 0),
	KEY(SLPT_K_BATTERY_CHARGING_CV, "battery-charging-cv", 0),
	KEY(SLPT_K_BATTERY_DISCHARGING_CV, "battery-discharging-cv", 0),
	KEY(SLPT_K_BATTERY_WARN_CAPACITY, "warn-capacity", 0),
	KEY(SLPT_K_RESERVE_MEM_ADDR, "reserve-mem-addr", CONFIG_SYS_SDRAM_BASE),
	KEY(SLPT_K_GO_KERNEL, "go-kernel", 1),
	KEY(SLPT_K_VOICE_TRIGGER_STATE, "voice-trigger-state", 0),
	KEY(SLPT_K_FRIZZ_PEDO, "frizz_pedo", 0),
	KEY(SLPT_K_FRIZZ_GESTURE, "frizz_gesture", 0),
	KEY(SLPT_K_SLEEP_MOTION, "sleep_motion", 0),
	KEY(SLPT_K_SLEEP_MOTION_ENABLE, "sleep_motion_enable", 0),
	KEY(SLPT_K_IOCTL, "ioctl", slpt_ioctl),
	KEY(SLPT_K_FB_ON, "fb_on", 1),
	KEY(SLPT_K_SAMPLE_ADC_FOR_KERNEL, "sample_adc_for_kernel", 0),
	KEY(SLPT_K_BOARD_NAME, "board_name", NULL),
};

/* this id is in slpt */
int slpt_set_key(unsigned int id, unsigned long val) {
	assert(id < SLPT_K_NUMS);

	if (slpt_key_list[id].id < 0) {
		return -ENODEV;
	} else {
		*slpt_key_list[id].val_addr = val;
		return 0;
	}
}

/* this id is in slpt
 * copy the default val if id valid but not exist in kernel
 */
int slpt_get_key(unsigned int id, unsigned long *val) {
	assert(id < SLPT_K_NUMS);

	if (slpt_key_list[id].id < 0) {
		*val = slpt_key_list[id].val;
		return -ENODEV;
	} else {
		*val = *slpt_key_list[id].val_addr;
		return 0;
	}
}

void slpt_key_init_all(void) {
	unsigned int i;
	struct slpt_key *key = slpt_key_list;

	for (i = 0; i < ARRAY_SIZE(slpt_key_list); ++i) {
		key[i].id = slpt_kernel_get_key_id(key[i].name, &key[i].val_addr);
		if (key[i].id >= 0) {
#ifdef CONFIG_M200
			key[i].val_addr = (void *)((unsigned long )key[i].val_addr | 0xa0000000); /* to kseg1 addr */
#endif
		}
		slpt_kernel_printf("%s : %d %x\n", key[i].name, key[i].id, key[i].val_addr);
	}
}

int slpt_key_debug_info(void) {
	pr_debug("SLPT_K_POWER_STATE          --> power-state         [%s]\n", slpt_kernel_get_power_state());
	pr_debug("SLPT_K_CHARGER_GPIO         --> charger-gpio        [%d]\n", slpt_kernel_get_charger_gpio());
	pr_debug("SLPT_K_CHARGER_LEVEL        --> charger-level       [%d]\n", slpt_kernel_get_charger_level());
	pr_debug("SLPT_K_LOW_BATTERY_WARN_VOL --> low-bat-warn-vol    [%d]\n", slpt_kernel_get_low_battery_warn_voltage());
	pr_debug("SLPT_K_BATTERY_VOLTAGE      --> battery-voltage     [%d]\n", slpt_kernel_get_battery_voltage());
	pr_debug("SLPT_K_BATTERY_LOW_VOLTAGE  --> battery-low-voltage [%d]\n", slpt_kernel_get_battery_low_voltage());
	pr_debug("SLPT_K_BATTERY_CAPACITY     --> battery-capacity    [%d]\n", slpt_kernel_get_battery_capacity());
	pr_debug("SLPT_K_BATTERY_WARN_CAPACITY--> warn-capacity       [%d]\n", slpt_kernel_get_warn_capacity());

	return 0;
}

static int slpt_register_key(struct slpt_app_res *key_dir,
                             unsigned int id, const char *name, unsigned int len) {
	struct slpt_app_res tmp;

	if (slpt_key_list[id].id < 0) {
		pr_err("slpt key: register: not found key [%s]\n", slpt_key_list[id].name);
		return -ENODEV;
	}

	tmp.type = SLPT_RES_MEM;
	tmp.name = name ? name : slpt_key_list[id].name;
	slpt_set_res(tmp, slpt_key_list[id].val_addr, len);

	assert(slpt_kernel_register_app_child_res(&tmp, key_dir) != NULL);

	return 0;
}

int slpt_key_init_onetime(void) {
	struct slpt_app_res *setting_dir = slpt_kernel_name_to_app_res("setting", uboot_slpt_task);
	struct slpt_app_res *key_dir = slpt_kernel_name_to_app_child_res("key", setting_dir);

	assert(key_dir);
	slpt_register_key(key_dir, SLPT_K_CHARGER_GPIO,          "charger-gpio",  sizeof(unsigned long));
	slpt_register_key(key_dir, SLPT_K_CHARGER_LEVEL,         "charger-level", sizeof(unsigned long));
	slpt_register_key(key_dir, SLPT_K_LOW_BATTERY_WARN_VOL,  "warn-voltage",  sizeof(unsigned long));
	slpt_register_key(key_dir, SLPT_K_BATTERY_LOW_VOLTAGE,   "low-voltage",   sizeof(unsigned long));
	slpt_register_key(key_dir, SLPT_K_BATTERY_WARN_CAPACITY, "warn-capacity", sizeof(unsigned long));

	slpt_key_debug_info();

	return 0;
}

SLPT_APP_INIT_ONETIME(slpt_key_init_onetime);
SLPT_APP_INIT_EVERYTIME(slpt_key_debug_info);
