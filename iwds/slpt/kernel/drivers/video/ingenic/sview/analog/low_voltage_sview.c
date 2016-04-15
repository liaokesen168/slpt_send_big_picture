#include <common.h>
#include <sview/sview.h>
#include <sview/low_voltage_sview.h>

#ifdef CONFIG_SLPT
struct slpt_app_res *slpt_register_low_voltage_sview(struct sview *view, struct slpt_app_res *parent)
{
	return slpt_register_pic_sview(view, parent);
}
#endif

void low_voltage_sview_draw(struct sview *view) {
	pic_sview_draw(view);
}


void low_voltage_sview_measure_size(struct sview *view) {
	pic_sview_measure_size(view);
}

int low_voltage_sview_sync(struct sview *view) {
	struct low_voltage_sview *lbv = to_low_voltage_sview(view);

	lbv->pic_ready = !pic_sview_sync(view);

	return (lbv->pic_ready && low_pmu_voltage_mode()) ? 0 : 1;
}

void low_voltage_sview_free(struct sview *view) {
	struct low_voltage_sview *lbv = to_low_voltage_sview(view);
	unsigned int is_alloc = view->is_alloc;

	unregister_low_voltage_notify(&lbv->no);

	view->is_alloc = 0;
	pic_sview_free(view);

	if(is_alloc)
		free(lbv);
}

static void low_voltage_sview_callback(struct low_voltage_notify *no) {
	struct low_voltage_sview *lbv = container_of(no, struct low_voltage_sview, no);

	if (lbv->pic_ready)
		to_sview(lbv)->ready = low_pmu_voltage_mode();
}

void init_low_voltage_sview(struct low_voltage_sview *lbv, const char *name) {
	init_pic_sview(&lbv->picv, name);

	to_sview(lbv)->is_alloc = 0;
	to_sview(lbv)->type = SVIEW_LOW_VOLTAGE;

	lbv->no.callback = low_voltage_sview_callback;
	register_low_voltage_notify(&lbv->no);
}

struct sview *alloc_low_voltage_sview(const char *name) {
	struct low_voltage_sview *lbv;
	char *cpy_name;

	lbv = malloc_with_name(sizeof(*lbv), name);
	if(!lbv) {
		pr_err("low_voltage_sview: failed to alloc!\n");
		return NULL;
	}
	cpy_name = (char *)&lbv[1];

	init_low_voltage_sview(lbv, cpy_name);

	to_sview(lbv)->is_alloc = 1;

	return to_sview(lbv);
}
