#ifndef _SLPT_H_
#define _SLPT_H_

enum slpt_res_type {
	SLPT_RES_FUNCTION = 0,
	SLPT_RES_INT,

	/* keep last */
	SLPT_RES_MEM,
	SLPT_RES_MAX,
};

struct slpt_app_res {
	const char *name;
	unsigned int type;
	void *addr;
	unsigned int length;
};

struct slpt_task ;
struct slpt_res ;

#define __force
typedef int suspend_state_t;

#define PM_SUSPEND_ON		((__force suspend_state_t) 0)
#define PM_SUSPEND_STANDBY	((__force suspend_state_t) 1)
#define PM_SUSPEND_MEM		((__force suspend_state_t) 3)
#define PM_SUSPEND_MAX		((__force suspend_state_t) 4)

extern unsigned long (*slpt_app_get_api_val)(const char *name, struct slpt_task *task);
extern unsigned long slpt_app_get_api(const char *name, struct slpt_task *task);

extern struct slpt_app_res *slpt_app_register_res(struct slpt_app_res *res, struct slpt_task *task);
extern void slpt_app_unregister_res(struct slpt_app_res *res, struct slpt_task *task);
struct slpt_app_res *name_to_slpt_app_res(const char *name, struct slpt_task *task);

extern int slpt_pm_enter(suspend_state_t state);

extern int (*slpt_printf_val) (const char *fmt, ...);

#define slpt_printf(args...)											\
	do {																\
		if (!slpt_printf_val) {											\
			slpt_printf_val = (void *)slpt_app_get_api("printf", NULL);	\
		}																\
		slpt_printf_val ? slpt_printf_val(args) : 0;				\
	} while (0);


#endif /* _SLPT_H_ */
