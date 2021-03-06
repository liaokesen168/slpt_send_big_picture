
#ifndef _YEAR_EN_VIEW_H_
#define _YEAR_EN_VIEW_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <view.h>
#include <current_time.h>
#include <time_notify.h>

#ifdef CONFIG_SLPT
#include <slpt.h>
#endif

struct year_en_view {
	struct text_view text;
	struct time_notify no;
	struct view *array[4];
	struct view *array2[1];

/*if the view-member of the function we need to re-define, we should store the parent's ,too. others we can persist it*/
	void (*parent_freev)(struct view *view);
};

#ifdef CONFIG_SLPT
extern struct slpt_app_res *slpt_register_text_view(struct view *view, struct slpt_app_res *parent);

static inline struct slpt_app_res *slpt_register_year_en(struct year_en_view *yearv,
                                                                  struct slpt_app_res *parent) {
	return slpt_register_view(&yearv->text.view, parent, NULL, 0);
}

#endif

#ifdef __cplusplus
}
#endif
#endif /* _YEAR_EN_VIEW_H_ */
