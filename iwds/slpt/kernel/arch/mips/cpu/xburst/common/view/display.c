#include <config.h>
#include <common.h>
#include <malloc.h>
#include <linux/err.h>
#include <command.h>
#include <slpt_app.h>
#include <slpt.h>

#include <slpt_clock.h>
#include <arg_parse.h>

extern char *strctoc(char *s, char c1, char c2);

#define MAX_VIEWS 8
#define MAX_DISPLAY_LEN (MAX_VIEWS * (MAX_NAME_LEN + 1))

struct display_setting {
	char clocks[MAX_DISPLAY_LEN];
	struct list_head handlers;
	struct list_head handlers2;
};

static struct display_setting display = {
	.handlers = LIST_HEAD_INIT(display.handlers),
	.handlers2 = LIST_HEAD_INIT(display.handlers2),
};

static struct slpt_app_res display_res[] = {
	SLPT_RES_ARR_DEF("clocks-to-show", display.clocks),
};

static struct slpt_app_res display_dir = SLPT_RES_DIR_DEF("display", display_res);

static void register_slpt_display(void) {
	struct slpt_app_res *setting_dir;

	setting_dir = slpt_kernel_name_to_app_res("setting", uboot_slpt_task);
	assert(setting_dir);

	assert(slpt_kernel_register_app_dir_res(&display_dir, setting_dir));
}

static int find_display_name(const char *name) {
	struct arg_parse ap;
	const char *arg;
	int ret = -1;

	arg_parse_init(&ap, display.clocks);
	while ((arg = arg_parse_next(&ap))) {
		pr_debug("display: str: [%s]\n", arg);
		if (!strcmp(arg, name)) {
			ret = 0;
			goto out;
		}
	}

out:
	arg_parse_destory(&ap);
	return ret;
}

static struct view *find_display_view(const char *name) {
	struct list_head *pos, *n;

	/* firstly we clean the dead fish */
	list_for_each_safe(pos, n, &display.handlers) {
		struct view *v = list_entry(pos, struct view, link);
		if (!strcmp(view_name(v), name))
			return v;
	}

	return NULL;
}

static struct view *find_freeze_view(const char *name) {
	struct list_head *pos, *n;

	/* firstly we clean the dead fish */
	list_for_each_safe(pos, n, &display.handlers2) {
		struct view *v = list_entry(pos, struct view, link);
		if (!strcmp(view_name(v), name))
			return v;
	}

	return NULL;
}

/* add to display view list */
static inline void add_display_view(struct view *v) {
	list_add_tail(&v->link, &display.handlers);
}

static void add_display_view_by_level(struct view *v) {
	struct list_head *pos;

	list_for_each(pos, &display.handlers) {
		struct view *view = list_entry(pos, struct view, link);
		pr_debug("scan: [%s]\n", view_name(view));
		if (view_level(v) < view_level(view))
			break;
	}
	list_add_tail(&v->link, pos);
}

/* add to freeze view list */
static inline void add_freeze_view(struct view *v) {
	list_add_tail(&v->link, &display.handlers2);
}

static void sync_view_list(void) {
	struct list_head *pos, *n;
	struct arg_parse ap;
	const char *arg;
	struct view *v;

	/* remove the trailing newline by "echo" */
	strctoc(display.clocks, '\n', '\0');

	pr_debug("display: clean dead fish\n");

	/* firstly we clean the dead fish, mv to freeze view list */
	list_for_each_safe(pos, n, &display.handlers) {
		struct view *v = list_entry(pos, struct view, link);
		if (find_display_name(view_name(v))) {
			list_del(&v->link);
			add_freeze_view(v);
		}
	}

	pr_debug("display: add new comer\n");
	/* add new comer, mv it from freeze view list to display view list */
	arg_parse_init(&ap, display.clocks);
	while ((arg = arg_parse_next(&ap))) {
		pr_debug("display: find: [%s] %s\n", arg, find_display_view(arg) ? "Y" : "N");
		if (!find_display_view(arg)) {
			v = find_freeze_view(arg);
			if (!v) {
				pr_err("display: can't find the view: [%s]\n", arg);
			} else {
				list_del(&v->link);
				add_display_view(v);
			}
		}
	}

	arg_parse_destory(&ap);
}

static void sort_view_list(void) {
	struct list_head *pos, *n;
	struct list_head list = LIST_HEAD_INIT(list);

	list_cut_position(&list, &display.handlers, display.handlers.prev);
	list_for_each_safe(pos, n, &list) {
		struct view *v = list_entry(pos, struct view, link);
		list_del(pos);
		add_display_view_by_level(v);
	}
}

static void sync_views(void) {
	struct list_head *pos, *n;

	pr_debug("display: sync every views\n");

	list_for_each_safe(pos, n, &display.handlers) {
		struct view *v = list_entry(pos, struct view, link);
		view_sync_setting(v);
	}

	sort_view_list();
}

static void sync_start_of_views(void) {
	struct list_head *pos, *n;

	list_for_each_safe(pos, n, &display.handlers) {
		struct view *v = list_entry(pos, struct view, link);
		view_sync_start(v);
	}
}

static void pre_display_views(void) {
	struct list_head *pos, *n;

	list_for_each_safe(pos, n, &display.handlers) {
		struct view *v = list_entry(pos, struct view, link);
		view_pre_display(v);
	}
}

static void display_views(void) {
	struct list_head *pos, *n;

	list_for_each_safe(pos, n, &display.handlers) {
		struct view *v = list_entry(pos, struct view, link);
		view_display(v);
	}
}

/**
 * add a view to slpt display
 * firstly add to freeze view list.
 */
void slpt_display_add_view(struct view *v) {
	add_freeze_view(v);
}

void slpt_display_free_view_list(void) {
	struct list_head *pos, *n;

	pr_debug("display: free whole list\n");

	list_for_each_safe(pos, n, &display.handlers) {
		struct view *v = list_entry(pos, struct view, link);
		free_view(v);
	}

	list_for_each_safe(pos, n, &display.handlers2) {
		struct view *v = list_entry(pos, struct view, link);
		free_view(v);
	}
}

void slpt_display_delete_all_view(void) {
	struct list_head *pos;
	struct list_head *list_tmp;
	struct view *v;

	list_for_each_safe(pos, list_tmp, &display.handlers) {
		v = list_entry(pos, struct view, link);
		pr_info("delete view name is (%s)\n", v->name);
		slpt_unregister_view(v);
		free_view(v);
	}

	list_for_each_safe(pos, list_tmp, &display.handlers2) {
		v = list_entry(pos, struct view, link);
		pr_info("delete view name is (%s)\n", v->name);
		slpt_unregister_view(v);
		free_view(v);
	}
}

void slpt_display_sync(void) {
	sync_global_bg();
	sync_view_list();
	sync_views();

	set_time_notify_level(TIME_TICK_YEAR);

	sync_low_battery();
	sync_charge_picture();
	sync_chargefull_picture();
}

void slpt_display(void) {
	display_global_bg();
	sync_start_of_views();
	time_notify();
	pre_display_views();
	display_views();
	display_low_battery();
	display_charge_picture();
	display_chargefull_picture();
}

void slpt_display_init(void) {
	register_slpt_display();
	init_global_bg_pic();
	slpt_init_analog_clock();
	slpt_init_analog_week_clock();
	slpt_init_analog_month_clock();
	slpt_init_analog_second_clock();
	slpt_init_analog_minute_clock();
	slpt_init_analog_hour_clock();
/*	slpt_init_digital_clock();*/
	slpt_init_date_en_view();
	slpt_init_date_cn_view();
	slpt_init_week_en_view();
	slpt_init_week_cn_view();
	slpt_init_year_en_view();
	slpt_init_time_view();
	init_low_battery();
	slpt_init_charge_picture();
	slpt_init_chargefull_picture();
}
