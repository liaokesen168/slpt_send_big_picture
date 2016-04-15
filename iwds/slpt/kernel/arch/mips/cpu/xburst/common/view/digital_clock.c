#include <config.h>
#include <common.h>
#include <malloc.h>
#include <linux/err.h>
#include <command.h>
#include <slpt_app.h>
#include <slpt.h>

#include <slpt_clock.h>

static struct digital_clock_en clocken;
static struct digital_clock_cn clockcn;

static void init_default_setting(void) {
	struct position time_start = {0x26, 0x38};
	struct position datecn_start = {0x39, 0x85};
	struct position weekcn_start = {0xa5, 0x85};
	struct position dateen_start = {0x70, 0x85};
	struct position weeken_start = {0x39, 0x85};

	timev_set_start(&clocken.timev, &time_start);
	dateenv_set_start(&clocken.datev, &dateen_start);
	weekenv_set_start(&clocken.weekv, &weeken_start);

	timev_set_start(&clockcn.timev, &time_start);
	datecnv_set_start(&clockcn.datev, &datecn_start);
	weekcnv_set_start(&clockcn.weekv, &weekcn_start);
}

int slpt_init_digital_clock(void) {
	struct slpt_app_res *clock_dir;

	clock_dir = slpt_kernel_name_to_app_res("clock", uboot_slpt_task);
	assert(clock_dir);

	assert(!init_digital_clock_en(&clocken, "digital-clock-en-viewb"));
	assert(slpt_register_digital_clock_en(&clocken, clock_dir));

	assert(!init_digital_clock_cn(&clockcn, "digital-clock-cn-viewc"));
	assert(slpt_register_digital_clock_cn(&clockcn, clock_dir));

	init_default_setting();

	slpt_display_add_view(&clocken.view);
	slpt_display_add_view(&clockcn.view);

	return 0;
}
