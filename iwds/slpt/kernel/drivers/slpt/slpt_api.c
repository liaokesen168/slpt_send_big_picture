#include <config.h>
#include <common.h>
#include <slpt.h>
#include <slpt_app.h>
#include <vsprintf.h>
#include <linux/pr_info.h>
#include <linux/err.h>

struct slpt_task *uboot_slpt_task;

unsigned long (*slpt_kernel_get_api_val)(const char *name, struct slpt_task *task) = NULL;
unsigned long slpt_kernel_get_api(const char *name, struct slpt_task *task) {
	if (!slpt_kernel_get_api_val) {
		pr_err("SLPT: error: %s has not been inited\n", __FUNCTION__);
		return 0;
	}
	return slpt_kernel_get_api_val(name, task);
}

#ifdef DEBUG
#define slpt_get_func(func, name, task)									\
	do {																\
		(func) =(typeof(*func) *) slpt_kernel_get_api(name, task);		\
	} while (0)
#else
#define slpt_get_func(func, name, task)									\
	do {																\
		(func) =(typeof(*func) *) slpt_kernel_get_api(name, task);		\
		pr_info("SLPT: info: %s : %p\n", __FUNCTION__, (func));			\
	} while (0)
#endif

static struct slpt_app_res* (*slpt_kernel_register_app_res_val)(struct slpt_app_res *res, struct slpt_task *task) = NULL;
struct slpt_app_res *slpt_kernel_register_app_res(struct slpt_app_res *res, struct slpt_task *task) {
	if (!slpt_kernel_register_app_res_val) {
		slpt_get_func(slpt_kernel_register_app_res_val, "res-register", task);
	}

	return slpt_kernel_register_app_res_val ? slpt_kernel_register_app_res_val(res, task) : NULL;
}

struct slpt_app_res *(*slpt_kernel_register_app_child_res_val)(struct slpt_app_res *res, struct slpt_app_res *p_res);
struct slpt_app_res *slpt_kernel_register_app_child_res(struct slpt_app_res *res, struct slpt_app_res *p_res) {
	if (!slpt_kernel_register_app_child_res_val) {
		slpt_get_func(slpt_kernel_register_app_child_res_val, "slpt_app_register_child_res", NULL);
	}

	return slpt_kernel_register_app_child_res_val ? slpt_kernel_register_app_child_res_val(res, p_res) : NULL;
}

static void (*slpt_kernel_unregister_app_res_val)(struct slpt_app_res *res, struct slpt_task *task) = NULL;
void slpt_kernel_unregister_app_res(struct slpt_app_res *res, struct slpt_task *task) {
	if (!slpt_kernel_unregister_app_res_val) {
		slpt_get_func(slpt_kernel_unregister_app_res_val, "res-unregister", task);
	}

	if (slpt_kernel_unregister_app_res_val)
		slpt_kernel_unregister_app_res_val(res, task);
}

static struct slpt_app_res *(*slpt_kernel_name_to_app_res_val)(const char *name, struct slpt_task *task) = NULL;
struct slpt_app_res *slpt_kernel_name_to_app_res(const char *name, struct slpt_task *task) {
	if (!slpt_kernel_name_to_app_res_val) {
		slpt_get_func(slpt_kernel_name_to_app_res_val, "res-get", task);
	}

	return slpt_kernel_name_to_app_res_val ? slpt_kernel_name_to_app_res_val(name, task) : NULL;
}

struct slpt_app_res *(*slpt_kernel_name_to_app_child_res_val)(const char *name, struct slpt_app_res *p_res);
struct slpt_app_res *slpt_kernel_name_to_app_child_res(const char *name, struct slpt_app_res *p_res) {
	if (!slpt_kernel_name_to_app_child_res_val) {
		slpt_get_func(slpt_kernel_name_to_app_child_res_val, "name_to_slpt_app_child_res", NULL);
	}
	return slpt_kernel_name_to_app_child_res_val ? slpt_kernel_name_to_app_child_res_val(name, p_res) : NULL;
}

static int (*slpt_kernel_pm_enter_val)(suspend_state_t state) = NULL;
int slpt_kernel_pm_enter(suspend_state_t state) {
	if (!slpt_kernel_pm_enter_val) {
		slpt_get_func(slpt_kernel_pm_enter_val, "pm-suspend-enter", NULL);
	}

	return slpt_kernel_pm_enter_val ? slpt_kernel_pm_enter_val(state) : -1;
}

static void (*slpt_kernel_dma_cache_sync_val)(unsigned long addr, unsigned long size) = NULL;
void slpt_kernel_dma_cache_sync(unsigned long addr, unsigned long size) {
	if (!slpt_kernel_dma_cache_sync_val) {
		slpt_get_func(slpt_kernel_dma_cache_sync_val, "kernel_dma_cache_sync", NULL);
	}

	if (slpt_kernel_dma_cache_sync_val)
		slpt_kernel_dma_cache_sync_val(addr, size);
}

int slpt_printf_kernel_mode = 1;

static int (*slpt_kernel_printf_val) (const char *fmt, ...) = NULL;
int slpt_kernel_printf(const char *fmt, ...) {
#define ARGS_LEN 200
	char buf[ARGS_LEN + 1];
	va_list args;
	int r = 0;

	if (!slpt_kernel_printf_val) {
		slpt_kernel_printf_val = (void *) slpt_kernel_get_api("printf", NULL);
	}

	if (slpt_kernel_printf_val) {
		va_start(args, fmt);
		r = vsnprintf(buf, sizeof(buf) - 1, fmt, args);
		va_end(args);
		if (slpt_printf_kernel_mode)
			slpt_kernel_printf_val("%s", buf);
		else
			printf ("%s\n", buf);
	}

	return r;
}

unsigned long (*slpt_kernel_get_jzfb_param_val)(int id, const char *param);
unsigned long slpt_kernel_get_jzfb_param(int id, const char *param) {
	if (!slpt_kernel_get_jzfb_param_val) {
		slpt_get_func(slpt_kernel_get_jzfb_param_val, "slpt_get_jzfb_param", NULL);
	}

	return slpt_kernel_get_jzfb_param_val ? slpt_kernel_get_jzfb_param_val(id, param) : 0;
}

struct client_i2c_data *(*slpt_kernel_get_i2c_bus_val)(int *size);
struct client_i2c_data *slpt_kernel_get_i2c_bus(int *size) {
	if (!slpt_kernel_get_i2c_bus_val) {
		slpt_get_func(slpt_kernel_get_i2c_bus_val, "slpt_kernel_get_i2c_bus", NULL);
	}

	return slpt_kernel_get_i2c_bus_val ? slpt_kernel_get_i2c_bus_val(size) : NULL;
}

const char *(*slpt_kernel_get_default_lcd_val)(int *idp);
const char *slpt_kernel_get_default_lcd(int *idp) {
	if (!slpt_kernel_get_default_lcd_val) {
		slpt_get_func(slpt_kernel_get_default_lcd_val, "slpt_get_default_lcd", NULL);
	}

	return slpt_kernel_get_default_lcd_val ? slpt_kernel_get_default_lcd_val(idp) : 0;
}

int (*slpt_kernel_get_key_id_val)(const char *name, unsigned long **val_addr);
int slpt_kernel_get_key_id(const char *name, unsigned long **val_addr) {
	if (!slpt_kernel_get_key_id_val) {
		slpt_get_func(slpt_kernel_get_key_id_val, "slpt_get_key_id", NULL);
	}

	return slpt_kernel_get_key_id_val ? slpt_kernel_get_key_id_val(name, val_addr) : -1;
}

int (*slpt_kernel_set_key_by_id_val)(int id, unsigned long val);
int slpt_kernel_set_key_by_id(int id, unsigned long val) {
	if (!slpt_kernel_set_key_by_id_val) {
		slpt_get_func(slpt_kernel_set_key_by_id_val, "slpt_set_key_by_id", NULL);
	}

	return slpt_kernel_set_key_by_id_val ? slpt_kernel_set_key_by_id_val(id, val) : -1;
}

int (*slpt_kernel_get_key_by_id_val)(int id, unsigned long *val);
int slpt_kernel_get_key_by_id(int id, unsigned long *val) {
	if (!slpt_kernel_get_key_by_id_val) {
		slpt_get_func(slpt_kernel_get_key_by_id_val, "slpt_get_key_by_id", NULL);
	}

	return slpt_kernel_get_key_by_id_val ? slpt_kernel_get_key_by_id_val(id, val) : -1;
}

int (*slpt_kernel_register_method_val)(struct slpt_task *task, const char *name, unsigned long addr);
int slpt_kernel_register_method(const char *name, unsigned long addr) {
	if (!slpt_kernel_register_method_val) {
		slpt_get_func(slpt_kernel_register_method_val, "slpt_register_method", NULL);
	}

	return slpt_kernel_register_method_val ? slpt_kernel_register_method_val(uboot_slpt_task, name, addr) : -1;
}

void (*slpt_kernel_unregister_method_val)(struct slpt_task *task, const char *name);
void slpt_kernel_unregister_method(const char *name) {
	if (!slpt_kernel_unregister_method_val) {
		slpt_get_func(slpt_kernel_unregister_method_val, "slpt_unregister_method", NULL);
	}

	if (slpt_kernel_unregister_method_val) slpt_kernel_unregister_method_val(uboot_slpt_task, name);
}

void (*slpt_adc_add_data_val)(unsigned int time, unsigned int voltage);
void slpt_kernel_adc_add_data(unsigned int time, unsigned int voltage) {
	if (!slpt_adc_add_data_val) {
		slpt_get_func(slpt_adc_add_data_val, "slpt_adc_add_data", NULL);
	}

	if (slpt_adc_add_data_val) slpt_adc_add_data_val(time, voltage);
}

void *(*slpt_adc_get_fifo_ring_val)(void);
void *slpt_kernel_adc_get_fifo_ring(void) {
	if (!slpt_adc_get_fifo_ring_val) {
		slpt_get_func(slpt_adc_get_fifo_ring_val, "slpt_adc_get_fifo_ring", NULL);
	}

	return slpt_adc_get_fifo_ring_val ? slpt_adc_get_fifo_ring_val() : NULL;
}

void (*dump_gpio_pin_val)(int port, int pin);
void slpt_kernel_dump_gpio_pin(int port, int pin) {
	if (!dump_gpio_pin_val) {
		slpt_get_func(dump_gpio_pin_val, "dump_gpio_pin", NULL);
	}

	if (dump_gpio_pin_val) dump_gpio_pin_val(port, pin);
}

int (*slpt_get_kernel_pixel_align_val)(void);
int slpt_get_kernel_pixel_align(void)
{
	if(!slpt_get_kernel_pixel_align_val) {
		slpt_get_func(slpt_get_kernel_pixel_align_val, "slpt_get_kernel_pixel_align", NULL);
	}

	return slpt_get_kernel_pixel_align_val ? slpt_get_kernel_pixel_align_val() : -1;
}

int (*slpt_get_suspend_time)(long *tv_sec, long *tv_nsec);
int slpt_kernel_get_suspend_time(long *tv_sec, long *tv_nsec)
{
	if(!slpt_get_suspend_time) {
		slpt_get_func(slpt_get_suspend_time, "slpt_get_suspend_time", NULL);
	}

	return slpt_get_suspend_time ? slpt_get_suspend_time(tv_sec, tv_nsec) : -1;
}

int (*slpt_get_pmu_irq_gpio_val)(void);
int slpt_kernel_get_pmu_irq_gpio(void)
{
	if(!slpt_get_pmu_irq_gpio_val) {
		slpt_get_func(slpt_get_pmu_irq_gpio_val, "slpt_get_pmu_irq_gpio", NULL);
	}

	return slpt_get_pmu_irq_gpio_val ? slpt_get_pmu_irq_gpio_val() : -1;
}

static struct slpt_app_res *slpt_register_res_list_internal( struct slpt_app_res *list, unsigned int length, struct slpt_app_res *parent) {
	int i;
	struct slpt_app_res *res;

	for (i = 0; i < length; ++i) {
		res = slpt_kernel_register_app_child_res(&list[i], parent);
		if (!res) {
			slpt_kernel_unregister_app_res_array(list, i);
			return NULL;
		}

		if (list[i].type == SLPT_RES_DIR && list[i].addr) {
			if (!slpt_register_res_list_internal(list[i].addr, list[i].length, res)) {
				slpt_kernel_unregister_app_res(res, uboot_slpt_task);
				slpt_kernel_unregister_app_res_array(list, i);
				return NULL;
			}
		}
	}

	return parent;
}

struct slpt_app_res *slpt_kernel_register_app_child_res_list(struct slpt_app_res *list, unsigned int length, struct slpt_app_res *parent) {
	if (!list || !parent || parent->type != SLPT_RES_DIR) {
		return NULL;
	}

	return slpt_register_res_list_internal(list, length, parent);
}

struct slpt_app_res *slpt_kernel_register_app_dir_res(struct slpt_app_res *res, struct slpt_app_res *parent) {
	if (res->type != SLPT_RES_DIR) {
		return NULL;
	}

	if (!parent)
		res = slpt_kernel_register_app_res(res, uboot_slpt_task);
	else
		res = slpt_kernel_register_app_child_res(res, parent);

	if (!res) {
		return NULL;
	}

	if (res->addr) {
		if (!slpt_register_res_list_internal(res->addr, res->length, res)) {
			slpt_kernel_unregister_app_res(res, uboot_slpt_task);
			return NULL;
		}
	}

	return res;
}

struct slpt_app_res *slpt_get_res_from_array(const char *name, struct slpt_app_res *res_arr, unsigned int len) {
	unsigned int i;

	if (!name) {
		return NULL;
	}

	for (i = 0; i < len; ++i) {
		if (!strcmp(name, res_arr[i].name)) {
			return &res_arr[i];
		}
	}
	return NULL;
}

int slpt_kernel_register_app_res_array(struct slpt_app_res *res_arr, unsigned int len) {
	struct slpt_app_res *res;
	unsigned int i;

	for (i = 0; i < len; ++i) {
		if (res_arr[i].type == SLPT_RES_DIR)
			res = slpt_kernel_register_app_dir_res(&res_arr[i], NULL);
		else
			res = slpt_kernel_register_app_res(&res_arr[i], uboot_slpt_task);
		if (!res) {
			slpt_kernel_unregister_app_res_array(res_arr, i);
			return -ENOMEM;
		}
	}
	return 0;
}

void slpt_kernel_unregister_app_res_array(struct slpt_app_res *res_arr, unsigned int len) {
	struct slpt_app_res *res;
	unsigned int i;

	for ( i = len; i != 0; --i) {
		res = slpt_kernel_name_to_app_res(res_arr[i - 1].name, uboot_slpt_task);
		if (res) {
			slpt_kernel_unregister_app_res(res, uboot_slpt_task);
		} else {
			slpt_kernel_printf("unregister :failed to get %s\n", res_arr[i - 1].name);
		}
	}
}

extern unsigned int __slpt_res_def_list_start;
extern unsigned int __slpt_res_def_list_end;

static int slpt_res_def_list_init(void) {
	void *start = (void *) &__slpt_res_def_list_start;
	void *end  = (void *)&__slpt_res_def_list_end;
	void *p;
	struct slpt_res_def_struct *res_def;
	int ret;

	for (p = start; p < end; p += sizeof(struct slpt_res_def_struct)) {
		res_def = p;
		ret = slpt_kernel_register_app_res_array(res_def->res_arr, res_def->len);
		if (ret) {
			pr_err("res-def-list: failed to register app res array: %s\n", res_def->len ? res_def->res_arr[0].name : "null");
		}
	}
	return 0;
}
SLPT_ARCH_INIT_ONETIME(slpt_res_def_list_init);

static int slpt_res_def_list_exit(void) {
	void *start = (void *)&__slpt_res_def_list_start;
	void *end  = (void *)&__slpt_res_def_list_end;
	void *p;
	struct slpt_res_def_struct *res_def;

	for (p = start; p < end; p += sizeof(struct slpt_res_def_struct)) {
		res_def = p;
		slpt_kernel_unregister_app_res_array(res_def->res_arr, res_def->len);
	}
	return 0;
}
SLPT_ARCH_EXIT_ONETIME(slpt_res_def_list_exit);
