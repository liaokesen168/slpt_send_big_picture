#ifndef _CHARGE_PICTURE_VIEW_H_
#define _CHARGE_PICTURE_VIEW_H_

#include <view.h>
#include <time_notify.h>

#ifdef CONFIG_SLPT
#include <slpt.h>
#endif

extern struct charge_pic global_charge_pic;
extern int slpt_charger_online; /* the flag of the charge flag */

extern struct charge_pic global_chargefull_pic;
extern int chargefull_flag; /* the flag of the charge full flag */

struct charge_pic {
	struct flash_pic_view fpicv;
};

extern int slpt_init_charge_picture(void);
extern int slpt_init_chargefull_picture(void);

static inline int sync_charge_picture(void) {

	int ret = 0 ;

	ret = view_sync_setting(&(global_charge_pic.fpicv.view));

	view_sync_start(&(global_charge_pic.fpicv.view));

	return ret;
}

static inline void display_charge_picture(void) {
	unsigned int on;

	on = ((slpt_charger_online) && (!chargefull_flag)) ? 1 : 0 ; /* charging and the capacity not full */

	flash_pic_view_set_display(&global_charge_pic.fpicv.view, on);

	view_display(&global_charge_pic.fpicv.view);
}

static inline int sync_chargefull_picture(void) {

	int ret = 0 ;

	ret = view_sync_setting(&(global_chargefull_pic.fpicv.view));

	view_sync_start(&(global_chargefull_pic.fpicv.view));

	return ret;
}

static inline void display_chargefull_picture(void) {
	unsigned int on;

	on = ((slpt_charger_online) && (chargefull_flag)) ? 1 : 0 ; /* charging and the capacity full */

	flash_pic_view_set_display(&global_chargefull_pic.fpicv.view, on);

	view_display(&global_chargefull_pic.fpicv.view);
}

#endif
