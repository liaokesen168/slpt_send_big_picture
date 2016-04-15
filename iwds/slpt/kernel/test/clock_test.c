#define DEBUG

#include <config.h>
#include <common.h>
#include <command.h>

#include <linux/string.h>
#include <malloc.h>

#include <view.h>
#include <digital_clock.h>
#include <analog_clock.h>
#include <slpt.h>
#include <slpt_app.h>

#define mrun_command_list(str) run_command_list(str, -1, 0)

static struct digital_clock_en clocken;
static struct digital_clock_cn clockcn;
static struct analog_clock clocka;

static int clock_init_onetime(void) {
	struct slpt_app_res *test_dir;
	struct fb_region *bg;

	bg = get_current_fb_region();
	assert(bg);

	test_dir = slpt_kernel_name_to_app_res("test", uboot_slpt_task);
	assert(test_dir);

	if (init_digital_clock_en(&clocken, "digital-clocken")) {
		pr_err("digital clocken: failed to init\n");
		return 0;
	}

	if (!slpt_register_digital_clock_en(&clocken, test_dir)) {
		pr_err("digital clocken: failed to register to slpt\n");
		return 0;
	}

	if (init_digital_clock_cn(&clockcn, "digital-clockcn")) {
		pr_err("digital clockcn: failed to init\n");
		return 0;
	}

	if (!slpt_register_digital_clock_cn(&clockcn, test_dir)) {
		pr_err("digital clockcn: failed to register to slpt\n");
		return 0;
	}

	if (init_analog_clock(&clocka, "analog-clocka")) {
		pr_err("analog clocka: failed to init\n");
		return 0;
	}

	if (!slpt_register_analog_clock(&clocka, test_dir)) {
		pr_err("analog clocka: failed to register to slpt\n");
		return 0;
	}

	return 0;
}
SLPT_APP_INIT_ONETIME(clock_init_onetime);

static int cmd_digital_clocken_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	static unsigned int hour = 23, min = 59, sec = 0;
	static unsigned int year = 2014, mon = 9, day = 10;
	unsigned long time;

	struct position pos[3] = {
		{0x26, 0x38},
		{0x70, 0x85},
		{0x39, 0x85},
	};

	if (argc == 2) {
		if (!strcmp(argv[1], "sync")) {
			sync_digital_clock_en(&clocken);
		}
	} else if (argc == 3) {
		/* week is depend on date */
	} else if (argc == 4) {
		if (!strcmp(argv[1], "set_date")) {
			mon = simple_strtoul(argv[2], NULL, 10);
			day = simple_strtoul(argv[3], NULL, 10);
		}
	} else if (argc == 5) {
		if (!strcmp(argv[1], "set_time")) {
			hour = simple_strtoul(argv[2], NULL, 10);
			min = simple_strtoul(argv[3], NULL, 10);
			sec = simple_strtoul(argv[4], NULL, 10);
		} else if (!strcmp(argv[1], "set_date")) {
			year = simple_strtoul(argv[2], NULL, 10);
			mon = simple_strtoul(argv[3], NULL, 10);
			day = simple_strtoul(argv[4], NULL, 10);
		}
	}

	time = mktime(year, mon, day, hour, min, sec);
	set_current_time(time);

	timev_set_start(&clocken.timev, &pos[0]);
	dateenv_set_start(&clocken.datev, &pos[1]);
	weekenv_set_start(&clocken.weekv, &pos[2]);

	display_digital_clock_en(&clocken);
	lcd_pan_display(get_default_fb(), 0);

	return 0;
}

U_BOOT_CMD(
	digital_clocken_test,	10,	10,	cmd_digital_clocken_test,
	"digital_clocken_test test for digital clocken\n",
	"digital_clocken_test sync\n"
	"digital_clocken_test set_date year mon day\n"
	"digital_clocken_test set_time hour min sec\n"
);

static int cmd_digital_clockcn_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	static unsigned int hour = 23, min = 59, sec = 0;
	static unsigned int year = 2014, mon = 9, day = 10;
	unsigned long time;

	struct position pos[3] = {
		{0x26, 0x38},
		{0x70, 0x85},
		{0x39, 0x85},
	};

	if (argc == 2) {
		if (!strcmp(argv[1], "sync")) {
			sync_digital_clock_cn(&clockcn);
		}
	} else if (argc == 3) {
		/* week is depend on date */
	} else if (argc == 4) {
		if (!strcmp(argv[1], "set_date")) {
			mon = simple_strtoul(argv[2], NULL, 10);
			day = simple_strtoul(argv[3], NULL, 10);
		}
	} else if (argc == 5) {
		if (!strcmp(argv[1], "set_time")) {
			hour = simple_strtoul(argv[2], NULL, 10);
			min = simple_strtoul(argv[3], NULL, 10);
			sec = simple_strtoul(argv[4], NULL, 10);
		} else if (!strcmp(argv[1], "set_date")) {
			year = simple_strtoul(argv[2], NULL, 10);
			mon = simple_strtoul(argv[3], NULL, 10);
			day = simple_strtoul(argv[4], NULL, 10);
		}
	}

	time = mktime(year, mon, day, hour, min, sec);
	set_current_time(time);

	timev_set_start(&clockcn.timev, &pos[0]);
	datecnv_set_start(&clockcn.datev, &pos[1]);
	weekcnv_set_start(&clockcn.weekv, &pos[2]);

	display_digital_clock_cn(&clockcn);
	lcd_pan_display(get_default_fb(), 0);

	return 0;
}

U_BOOT_CMD(
	digital_clockcn_test,	10,	10,	cmd_digital_clockcn_test,
	"digital_clockcn_test test for digital clockcn\n",
	"digital_clockcn_test sync\n"
	"digital_clockcn_test set_date year mon day\n"
	"digital_clockcn_test set_time hour min sec\n"
);

static int run_digital_clock_test(int argc, char * const argv[]) {
	char buffer[500] = "digital_clockcn_test ";
	char *p = buffer + strlen(buffer);
	unsigned int i;
	unsigned int n;

	for (i = 1; i < argc; ++i) {
		n = sprintf(p, "%s ", argv[i]);
		p += n;
	}



	pr_info("CMD: %s\n", buffer);

	return mrun_command_list(buffer);
}

static int cmd_analog_clocka_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	static unsigned int hour = 23, min = 59, sec = 0;
	static unsigned int year = 2014, mon = 9, day = 10;
	unsigned long time;

	if (argc == 2) {
		if (!strcmp(argv[1], "sync")) {
			sync_analog_clock(&clocka);
		}
	} else if (argc == 3) {
		/* week is depend on date */
	} else if (argc == 4) {
		if (!strcmp(argv[1], "set_date")) {
			mon = simple_strtoul(argv[2], NULL, 10);
			day = simple_strtoul(argv[3], NULL, 10);
		}
	} else if (argc == 5) {
		if (!strcmp(argv[1], "set_time")) {
			hour = simple_strtoul(argv[2], NULL, 10);
			min = simple_strtoul(argv[3], NULL, 10);
			sec = simple_strtoul(argv[4], NULL, 10);
		} else if (!strcmp(argv[1], "set_date")) {
			year = simple_strtoul(argv[2], NULL, 10);
			mon = simple_strtoul(argv[3], NULL, 10);
			day = simple_strtoul(argv[4], NULL, 10);
		}
	}

	time = mktime(year, mon, day, hour, min, sec);
	set_current_time(time);

	restore_analog_clock(&clocka);

	run_digital_clock_test(argc, argv);

	save_and_draw_analog_clock(&clocka);

	lcd_pan_display(get_default_fb(), 0);

	return 0;
}

U_BOOT_CMD(
	analog_clocka_test,	10,	10,	cmd_analog_clocka_test,
	"analog_clocka_test test for analog clocka\n",
	"analog_clocka_test sync\n"
	"analog_clocka_test set_date year mon day\n"
	"analog_clocka_test set_time hour min sec\n"
);
