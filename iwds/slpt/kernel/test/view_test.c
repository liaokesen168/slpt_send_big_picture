#define DEBUG

#include <config.h>
#include <common.h>
#include <command.h>

#include <linux/string.h>
#include <malloc.h>

#include <view.h>
#include <slpt.h>
#include <slpt_app.h>

static struct picture_desc pics[10] = {
	{"0", 8192},
	{"1", 8192},
	{"2", 8192},
	{"3", 8192},
	{"4", 8192},
	{"5", 8192},
	{"6", 8192},
	{"7", 8192},
	{"8", 8192},
	{"9", 8192},
};

static struct view *nv;
static struct view *text;
static struct view *views[4];
int view_test_init_onetime(void) {
	unsigned int i;
	struct slpt_app_res *test_dir;
	struct fb_region *bg;

	bg = get_current_fb_region();
	assert(bg);

	for (i = 0; i < 10; ++i) {
		slpt_kernel_printf("--> %s\n", pics[i].name);
	}

	test_dir = slpt_kernel_name_to_app_res("test", uboot_slpt_task);

	assert(test_dir);

	if (!alloc_picture_grp("middle_nums", pics, ARRAY_SIZE(pics))) {
		slpt_kernel_printf("view test: failed to alloc middle nums\n");
		return 0;
	}

	if (!alloc_picture_grp("middle_nums_link", pics, ARRAY_SIZE(pics))) {
		slpt_kernel_printf("view test: failed to alloc middle nums link\n");
		return 0;
	}

	nv = alloc_num_view("test-numview", "middle_nums");
	if (!nv) {
		slpt_kernel_printf("view test: failed to alloc test-numview\n");
		return 0;
	}

	if (!slpt_register_view(nv, test_dir, NULL, 0)) {
		slpt_kernel_printf("view test: failed to register test-numview to test dir\n");
		return 0;
	}

	view_set_bg(nv, bg);

	views[0] = alloc_num_view("dayl", "middle_nums");
	views[1] = alloc_num_view("dayh", "middle_nums");
	views[2] = alloc_num_view("monl", "middle_nums");
	views[3] = alloc_num_view("monh", "middle_nums");

	if (!views[0] || !views[1] || !views[2] || !views[3]) {
		slpt_kernel_printf("view test: failed to alloc views\n");
		return 0;
	}

	text = alloc_text_view("date", views, ARRAY_SIZE(views));
	if (!text) {
		slpt_kernel_printf("view test: failed to alloc text view\n");
		return 0;
	}

	if (!slpt_register_view(text, test_dir, NULL, 0)) {
		slpt_kernel_printf("view test: failed to register date to test dir\n");
		return 0;
	}

	view_set_bg(text, bg);

	return 0;
}
SLPT_APP_INIT_ONETIME(view_test_init_onetime);

#if 1
static int cmd_num_view_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	struct position pos = {0, 0};
	unsigned int num;

	if (argc == 2) {
		if (!strcmp(argv[1], "sync")) {
			view_sync_setting(nv);
		}
	} else if (argc == 3) {
		if (!strcmp(argv[1], "set_num")) {
			num = simple_strtol(argv[2], NULL, 10);
			num_view_set_num(nv, num);
		}
	} else if (argc == 4) {
		if (!strcmp(argv[1], "set_start")) {
			pos.x = simple_strtol(argv[2], NULL, 10);
			pos.y = simple_strtol(argv[3], NULL, 10);
			view_set_start(nv, &pos);
		}
	}

	view_display(nv);
	lcd_pan_display(get_default_fb(), 0);

	return 0;
}

U_BOOT_CMD(
	num_view_test,	10,	10,	cmd_num_view_test,
	"",
	""
);

static int cmd_text_view_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	struct position pos = {0, 0};
	unsigned int mon, date;
	int follow;

	if (argc == 2) {
		if (!strcmp(argv[1], "sync")) {
			view_sync_setting(text);
		}
	} else if (argc == 3) {
		if (!strcmp(argv[1], "set_follow")) {
			follow = simple_strtol(argv[2], NULL, 10);
			view_set_follow(text, follow);
		}
	} else if (argc == 4) {
		if (!strcmp(argv[1], "set_date")) {
			mon = simple_strtol(argv[2], NULL, 10);
			date = simple_strtol(argv[3], NULL, 10);
			num_view_set_num(views[0], mon / 10);
			num_view_set_num(views[1], mon % 10);
			num_view_set_num(views[2], date / 10);
			num_view_set_num(views[3], date % 10);
		} else if (!strcmp(argv[1], "set_start")) {
			pos.x = simple_strtol(argv[2], NULL, 10);
			pos.y = simple_strtol(argv[3], NULL, 10);
			view_set_start(text, &pos);
		}
	}

	view_display(text);
	lcd_pan_display(get_default_fb(), 0);

	return 0;
}

U_BOOT_CMD(
	text_view_test,	10,	10,	cmd_text_view_test,
	"",
	""
);
#endif
