#include "types.h"
#include "slpt.h"

#define pr_err(args...) slpt_printf(args)
#define pr_info(args...) slpt_printf(args)
#ifdef DEBUG
#define pr_debug(args...) slpt_printf(args)
#else
#define pr_debug(args...) do {} while (0)
#endif

int (*slpt_printf_val) (const char *fmt, ...) = NULL;

unsigned long (*slpt_app_get_api_val)(const char *name, struct slpt_task *task) = NULL;
unsigned long slpt_app_get_api(const char *name, struct slpt_task *task) {
	if (!slpt_app_get_api_val) {
		pr_err("SLPT: error: %s has not been inited\n", __FUNCTION__);
		return 0;
	}
	return slpt_app_get_api_val(name, task);
}

#define slpt_get_func(func, name, task)								\
	do {															\
		(func) =(typeof(*func) *) slpt_app_get_api(name, task);		\
		slpt_printf("SLPT: info: %s : %p\n", __FUNCTION__, (func));	\
	} while (0)

static struct slpt_app_res* (*slpt_app_register_res_val)(struct slpt_app_res *res, struct slpt_task *task) = NULL;
struct slpt_app_res *slpt_app_register_res(struct slpt_app_res *res, struct slpt_task *task) {
	if (!slpt_app_register_res_val) {
		slpt_get_func(slpt_app_register_res_val, "res-register", task);
	}

	return slpt_app_register_res_val ? slpt_app_register_res_val(res, task) : NULL;
}

static void (*slpt_app_unregister_res_val)(struct slpt_app_res *res, struct slpt_task *task) = NULL;
void slpt_app_unregister_res(struct slpt_app_res *res, struct slpt_task *task) {
	if (!slpt_app_unregister_res_val) {
		slpt_get_func(slpt_app_unregister_res_val, "res-unregister", task);
	}

	return slpt_app_unregister_res_val ?  slpt_app_unregister_res_val(res, task) : NULL;
}

static struct slpt_app_res *(*name_to_slpt_app_res_val)(const char *name, struct slpt_task *task) = NULL;
struct slpt_app_res *name_to_slpt_app_res(const char *name, struct slpt_task *task) {
	if (!name_to_slpt_app_res_val) {
		slpt_get_func(name_to_slpt_app_res_val, "res-get", task);
	}

	return name_to_slpt_app_res_val ? name_to_slpt_app_res_val(name, task) : NULL;
}

static int (*slpt_pm_enter_val)(suspend_state_t state) = NULL;
int slpt_pm_enter(suspend_state_t state) {
	if (!slpt_pm_enter_val) {
		slpt_get_func(slpt_pm_enter_val, "pm-suspend-enter", NULL);
	}

	return slpt_pm_enter_val ? slpt_pm_enter_val(state) : -1;
}
