#ifndef _ANALOG_LOW_VOLTAGE_SVIEW_H_
#define _ANALOG_LOW_VOLTAGE_SVIEW_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <sview/sview_base.h>
#include <sview/pic_sview.h>
#include <slpt_app_low_voltage_detect.h>

#ifdef CONFIG_SLPT
#include <slpt.h>
#endif

struct low_voltage_sview {
	struct pic_sview picv;
	struct low_voltage_notify no;
	unsigned int pic_ready;
};

#define to_low_voltage_sview(view) ((struct low_voltage_sview *) (view))

#ifdef CONFIG_SLPT
extern struct slpt_app_res *slpt_register_low_voltage_sview(struct sview *view, struct slpt_app_res *parent);
#endif

extern void low_voltage_sview_draw(struct sview *view);
extern void low_voltage_sview_measure_size(struct sview *view);
extern int low_voltage_sview_sync(struct sview *view);
extern void low_voltage_sview_free(struct sview *view);
extern void init_low_voltage_sview(struct low_voltage_sview *lbv, const char *name);
extern struct sview *alloc_low_voltage_sview(const char *name);

#ifdef __cplusplus
}
#endif
#endif
