#ifndef _SLPT_H_
#define _SLPT_H_

#include <asm/arch/ioctl.h>

/* Define pm_entry function argument */
#define SLPTARG 'S'
#define SLPTARG_LOAD_RESETDLL  _IO(SLPTARG, 0x1)

#define SLPTARG_MODE_ACTIVE    _IO(SLPTARG, 0x2)
#define SLPTARG_MODE_LOWPOWER  _IO(SLPTARG, 0x3)
#define SLPTARG_MODE_WARNING   _IO(SLPTARG, 0x4)
#define SLPTARG_MODE_NO_CAPCITY  _IO(SLPTARG, 0x5)

extern unsigned int get_slpt_run_mode(void);
extern int set_slpt_run_mode(unsigned int);

enum slpt_res_type {
	SLPT_RES_FUNCTION = 0,
	SLPT_RES_INT,
	SLPT_RES_MEM,
	SLPT_RES_DIR,

	/* keep last */
	SLPT_RES_MAX,
};

struct slpt_app_res {
	const char *name;
	unsigned int type;
	void *addr;
	unsigned int length;
};

#define SLPT_RES_ARR_DEF(res_name, arry)         \
    {                                            \
        .name = res_name,                        \
        .type = SLPT_RES_MEM,                    \
        .addr = arry,                            \
        .length = sizeof(arry),                  \
    }

#define SLPT_RES_INT_DEF(res_name, num)          \
    {                                            \
        .name = res_name,                        \
        .type = SLPT_RES_INT,                    \
        .addr = &num,                            \
        .length = sizeof(num),                   \
    }

#define SLPT_RES_DIR_DEF(res_name, list)         \
    {                                            \
        .name = res_name,                        \
        .type = SLPT_RES_DIR,                    \
        .addr = list,                            \
        .length = ARRAY_SIZE(list),               \
    }

#define SLPT_RES_EMPTY_DEF(res_name, type_t)     \
    {                                            \
        .name = res_name,                        \
        .type = type_t,                          \
        .addr = NULL,                            \
        .length = 0,                             \
    }

#define slpt_set_res(res, p, len)               \
    do {                                        \
        (res).addr = (p);                       \
        (res).length = (len);                   \
    } while (0)

struct slpt_res_def_struct {
	struct slpt_app_res *res_arr;
	unsigned int len;
};

#define SLPT_RES_ARR_DEF_REGISTER(name, res_name, arry)                   \
    static struct slpt_app_res res_##name = SLPT_RES_ARR_DEF(res_name, arry); \
    __attribute__ ((__used__, __section__(".slpt_res_def_list")))         \
    static struct slpt_res_def_struct res_def##name = {                   \
        .res_arr = &(res_##name),                                         \
        .len = 1,                                                         \
    }

#define SLPT_RES_INT_DEF_REGISTER(name, res_name, num)                    \
    static struct slpt_app_res res_##name = SLPT_RES_INT_DEF(res_name, num); \
    __attribute__ ((__used__, __section__(".slpt_res_def_list")))         \
    static struct slpt_res_def_struct res_def##name = {                   \
        .res_arr = &(res_##name),                                         \
        .len = 1,                                                         \
    }

#define SLPT_REGISTER_RES_ARR(name, arry)                            \
    __attribute__ ((__used__, __section__(".slpt_res_def_list")))    \
    static struct slpt_res_def_struct res_def##name = {              \
        .res_arr = arry,                                             \
        .len = ARRAY_SIZE(arry),                                      \
    }

#define SLPT_REGISTER_RES(name, res)                                 \
    __attribute__ ((__used__, __section__(".slpt_res_def_list")))    \
    static struct slpt_res_def_struct res_def##name = {              \
        .res_arr = &(res),                                           \
        .len = 1,                                                    \
    }

struct slpt_task ;
struct slpt_res ;

#define __force
typedef int suspend_state_t;

#define PM_SUSPEND_ON		((__force suspend_state_t) 0)
#define PM_SUSPEND_STANDBY	((__force suspend_state_t) 1)
#define PM_SUSPEND_MEM		((__force suspend_state_t) 3)
#define PM_SUSPEND_MAX		((__force suspend_state_t) 4)

enum slpt_key_id {
	SLPT_K_POWER_STATE = 0,
	SLPT_K_CHARGER_GPIO,
	SLPT_K_CHARGER_LEVEL,
	SLPT_K_LOW_BATTERY_WARN_VOL, /* low battery low voltage is using old api: slpt_kernel_get_battery_low_voltage */
	SLPT_K_BATTERY_VOLTAGE,
	SLPT_K_BATTERY_LOW_VOLTAGE,
	SLPT_K_BATTERY_CAPACITY,
	SLPT_K_BATTERY_WAKE_FLAG,
	SLPT_K_BATTERY_CHARGING_CV,
	SLPT_K_BATTERY_DISCHARGING_CV,
	SLPT_K_BATTERY_WARN_CAPACITY,
	SLPT_K_RESERVE_MEM_ADDR,
	SLPT_K_GO_KERNEL,
	SLPT_K_VOICE_TRIGGER_STATE,
	SLPT_K_FRIZZ_PEDO,
	SLPT_K_FRIZZ_GESTURE,
	SLPT_K_SLEEP_MOTION,
	SLPT_K_SLEEP_MOTION_ENABLE,
	SLPT_K_IOCTL,
	SLPT_K_FB_ON,
	SLPT_K_SAMPLE_ADC_FOR_KERNEL,
	SLPT_K_BOARD_NAME,

	/* keep last */
	SLPT_K_NUMS,
};

#define SLPT_V_LOW_POWER_SHUTDOWN "low-power-shutdown"
#define SLPT_V_POWER_NORMAL "power-normal"

extern struct slpt_task *uboot_slpt_task;

extern unsigned long (*slpt_kernel_get_api_val)(const char *name, struct slpt_task *task);
extern unsigned long slpt_kernel_get_api(const char *name, struct slpt_task *task);

extern struct slpt_app_res *slpt_kernel_register_app_res(struct slpt_app_res *res, struct slpt_task *task);
extern  struct slpt_app_res *slpt_kernel_register_app_child_res(
	struct slpt_app_res *res, struct slpt_app_res *p_res);
extern void slpt_kernel_unregister_app_res(struct slpt_app_res *res, struct slpt_task *task);
extern struct slpt_app_res *slpt_kernel_name_to_app_res(const char *name, struct slpt_task *task);
struct slpt_app_res *slpt_kernel_name_to_app_child_res(const char *name, struct slpt_app_res *p_res);

extern int slpt_kernel_pm_enter(suspend_state_t state);
extern int slpt_kernel_printf(const char *fmt, ...);

extern unsigned long slpt_kernel_get_jzfb_param(int id, const char *param);
extern const char *slpt_kernel_get_default_lcd(int *idp);
extern void slpt_kernel_dma_cache_sync(unsigned long addr, unsigned long size);

extern struct slpt_app_res *slpt_get_res_from_array(
	const char *name, struct slpt_app_res *res_arr, unsigned int len);
extern void slpt_kernel_unregister_app_res_array(struct slpt_app_res *res_arr, unsigned int len);
extern int slpt_kernel_register_app_res_array(struct slpt_app_res *res_arr, unsigned int len);
extern struct slpt_app_res *slpt_kernel_register_app_child_res_list(
	struct slpt_app_res *list, unsigned int length, struct slpt_app_res *parent);
extern struct slpt_app_res *slpt_kernel_register_app_dir_res(
	struct slpt_app_res *res, struct slpt_app_res *parent);

extern struct client_i2c_data *slpt_kernel_get_i2c_bus(int *size);

extern int slpt_kernel_get_key_id(const char *name, unsigned long **val_addr); /* only called by slpt_key.c */
extern int slpt_kernel_set_key_by_id(int id, unsigned long val);
extern int slpt_kernel_get_key_by_id(int id, unsigned long *val);

extern int slpt_get_capacity(unsigned int voltage, unsigned int is_charging);

extern int slpt_set_key(unsigned int id, unsigned long val);
extern int slpt_get_key(unsigned int id, unsigned long *val);

extern int slpt_kernel_register_method(const char *name, unsigned long addr);
extern void slpt_kernel_unregister_method(const char *name);

extern void slpt_kernel_adc_add_data(unsigned int time, unsigned int voltage);
extern void *slpt_kernel_adc_get_fifo_ring(void);

extern void slpt_kernel_dump_gpio_pin(int port, int pin);

extern int slpt_get_kernel_pixel_align(void);

extern int slpt_kernel_get_suspend_time(long *tv_sec, long *tv_nsec);

extern int slpt_kernel_get_pmu_irq_gpio(void);

#define SLPT_SET_KEY(id, val) slpt_set_key(id, (unsigned long)(val))
#define SLPT_GET_KEY(id, val)                            \
    do {                                                 \
        unsigned long __tmp_val;                         \
        slpt_get_key(id, &__tmp_val);                    \
        *(val) = (typeof(*(val))) __tmp_val;             \
                                                         \
    } while (0)

/*
 * power state
 */
static inline void slpt_kernel_set_power_state(const char *state) {
	SLPT_SET_KEY(SLPT_K_POWER_STATE, state);
}

static inline const char *slpt_kernel_get_power_state(void) {
	const char *state;

	SLPT_GET_KEY(SLPT_K_POWER_STATE, &state);
	return state;
}

/*
 * charger gpio
 */
static inline void slpt_kernel_set_charger_gpio(int gpio) {
	SLPT_SET_KEY(SLPT_K_CHARGER_GPIO, gpio);
}

static inline int slpt_kernel_get_charger_gpio(void) {
	int gpio;

	SLPT_GET_KEY(SLPT_K_CHARGER_GPIO, &gpio);
	return gpio;
}

/*
 * charger gpio level
 */
static inline void slpt_kernel_set_charger_level(int level) {
	SLPT_SET_KEY(SLPT_K_CHARGER_LEVEL, level);
}

static inline int slpt_kernel_get_charger_level(void) {
	int level;

	SLPT_GET_KEY(SLPT_K_CHARGER_LEVEL, &level);
	return level;
}

/*
 * warn capacity
 */
static inline void slpt_kernel_set_warn_capacity(int warn_capacity) {
	SLPT_SET_KEY(SLPT_K_BATTERY_WARN_CAPACITY, warn_capacity);
}

static inline int slpt_kernel_get_warn_capacity(void) {
	int warn_capacity;

	SLPT_GET_KEY(SLPT_K_BATTERY_WARN_CAPACITY, &warn_capacity);
	return warn_capacity;
}

/*
 * battery
 */
static inline void slpt_kernel_set_battery_voltage(unsigned int voltage) {
	SLPT_SET_KEY(SLPT_K_BATTERY_VOLTAGE, voltage);
}

static inline unsigned int slpt_kernel_get_battery_voltage(void) {
	unsigned int voltage;

	SLPT_GET_KEY(SLPT_K_BATTERY_VOLTAGE, &voltage);
	return voltage;
}

static inline unsigned int slpt_kernel_get_battery_low_voltage(void) {
	unsigned int voltage;

	SLPT_GET_KEY(SLPT_K_BATTERY_LOW_VOLTAGE, &voltage);
	return voltage;
}

static inline unsigned int slpt_kernel_get_low_battery_warn_voltage(void) {
	unsigned int voltage;

	SLPT_GET_KEY(SLPT_K_LOW_BATTERY_WARN_VOL, &voltage);
	return voltage;
}

static inline int slpt_kernel_get_battery_capacity(void) {
	unsigned int capacity;

	SLPT_GET_KEY(SLPT_K_BATTERY_CAPACITY, &capacity);
	return capacity;
}

static inline void slpt_kernel_set_battery_capacity(unsigned int capacity) {
	SLPT_SET_KEY(SLPT_K_BATTERY_CAPACITY, capacity);
}

static inline unsigned int slpt_kernel_get_battery_wake_flag(void) {
	unsigned int flag;

	SLPT_GET_KEY(SLPT_K_BATTERY_WAKE_FLAG, &flag);
	return flag;
}

static inline void slpt_kernel_set_battery_wake_flag(unsigned int flag) {
	SLPT_SET_KEY(SLPT_K_BATTERY_WAKE_FLAG, flag);
}

static inline int *slpt_kernel_get_battery_charging_cv(void) {
	int *cv;

	SLPT_GET_KEY(SLPT_K_BATTERY_CHARGING_CV, &cv);
	return cv;
}

static inline int *slpt_kernel_get_battery_discharging_cv(void) {
	int *cv;

	SLPT_GET_KEY(SLPT_K_BATTERY_DISCHARGING_CV, &cv);
	return cv;
}

/*
 * reserve memory addr for slpt
 */
static inline unsigned long slpt_kernel_get_reserve_mem(void) {
	unsigned long addr;

	SLPT_GET_KEY(SLPT_K_RESERVE_MEM_ADDR, &addr);
	return addr;
}

/*
 * go back to kernel or sleep again
 */
static inline void slpt_kernel_set_go_kernel(unsigned int enable) {
	SLPT_SET_KEY(SLPT_K_GO_KERNEL, enable);
}

/*
 * voice trigger is enabled
 */
static inline int slpt_kernel_get_voice_trigger_state(void) {
	unsigned int enabled;

	SLPT_GET_KEY(SLPT_K_VOICE_TRIGGER_STATE, &enabled);
	return enabled;
}

/*
 * slpt ioctl
 */
static inline void *slpt_get_ioctl(void) {
	void *address;

	SLPT_GET_KEY(SLPT_K_IOCTL, &address);
	return address;
}

static inline void slpt_set_ioctl(void *address) {
	SLPT_SET_KEY(SLPT_K_IOCTL, address);
}

/*
 * fb on state: kernel or user want the fb on or not
 */
static inline int slpt_kernel_get_fb_on(void) {
	int fb_on;

	SLPT_GET_KEY(SLPT_K_FB_ON, &fb_on);
	return fb_on;
}

static inline void slpt_kernel_set_fb_on(int fb_on) {
	SLPT_SET_KEY(SLPT_K_FB_ON, fb_on);
}

/*
 * sample adc for kernel
 */
static inline int slpt_kernel_get_sample_adc_for_kernel(void) {
	int yes_or_no;

	SLPT_GET_KEY(SLPT_K_SAMPLE_ADC_FOR_KERNEL, &yes_or_no);
	return yes_or_no;
}

static inline void slpt_kernel_set_sample_adc_for_kernel(int yes_or_no) {
	SLPT_SET_KEY(SLPT_K_SAMPLE_ADC_FOR_KERNEL, yes_or_no);
}

/*
 * board name
 */
static inline const char *slpt_kernel_get_board_name(void) {
	const char *name;

	SLPT_GET_KEY(SLPT_K_BOARD_NAME, &name);
	return name;
}

static inline void slpt_kernel_set_board_name(const char *name) {
	SLPT_SET_KEY(SLPT_K_BOARD_NAME, name);
}

extern int slpt_low_power_shutdown; /* indicate slpt is in low power shutdown mode or not */
extern int slpt_charger_online; /* indicate charger is online or not */
extern int slpt_printf_kernel_mode;

extern int slpt_ioctl(void *hdr, unsigned hdr_len, void *mem, unsigned int mem_len, unsigned int cmd);

#endif /* _SLPT_H_ */
