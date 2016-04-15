#include <common.h>
#include <sview/sview.h>

void slpt_display_sync(void) {
	root_sview_sync_setting();
	set_time_notify_level(TIME_TICK_YEAR);
}

void slpt_display(void) {
	pr_debug("slpt_display_sview : time tick\n");
	time_notify();
	root_sview_measure_size();
	root_sview_draw();
}

struct sview *secondl = NULL;
struct sview *secondh = NULL;
struct sview *linear_layout = NULL;

void slpt_display_init(void) {
	secondl = alloc_secondL_sview("secondl");
	secondL_sview_set_pic_grp(secondl, "time");

	secondh = alloc_secondH_sview("secondh");
	secondH_sview_set_pic_grp(secondh, "time");

	linear_layout = alloc_linear_layout("linear-layout");
	linear_layout->background.color = 0xffffff;
	linear_layout_set_orientation(linear_layout, HORIZONTAL);
	linear_layout->align_x = ALIGN_CENTER;

	linear_layout_add(linear_layout, secondl);
	linear_layout_add(linear_layout, secondh);

	set_root_sview(linear_layout);
}
