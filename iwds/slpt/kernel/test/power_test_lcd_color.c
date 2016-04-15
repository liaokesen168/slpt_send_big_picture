/*
 * Copyright (C) 2014 Ingenic Semiconductor Co., Ltd.
 * Authors: Wu jiao  <jwu@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the License, or (at your
 * option) any later version.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#define DEBUG

#include <config.h>
#include <common.h>
#include <command.h>

#include <malloc.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/pr_info.h>

#include <fb_struct.h>

#define mrun_command_list(str) run_command_list(str, -1, 0)

int do_lcd_color_cmd(unsigned int hor, unsigned int color1, unsigned int color2) {
	char cmd_buf[50];

	sprintf(cmd_buf, "lcd_color hor %d 0x%x 0x%x\n", hor, color1, color2);

	mrun_command_list("lcd_power_on");
	mrun_command_list(cmd_buf);
	mrun_command_list("lcd_power_off");
	mdelay(10);
	if (ctrlc())
		return 1;
	mdelay(10);

	mrun_command_list("suspend");
	return 0;
}

static int cmd_power_test_lcd_color(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int color1 = 0xffffffff, color2 = 0x00;
	struct fb_struct *fbs = get_default_fb();
	unsigned int step = 10, start = 0, end = fbs->xres;
	unsigned int total = fbs->xres;
	unsigned int i, cnt;
	int ret;

	if (argc == 6 ) {
		color1 = simple_strtoul(argv[1], NULL, 16);
		color2 = simple_strtoul(argv[2], NULL, 16);
		step = simple_strtoul(argv[3], NULL, 10);
		start = simple_strtoul(argv[4], NULL, 10);
		end = simple_strtoul(argv[5], NULL, 10);
	} else {
		pr_info ("Usage: power_test_lcd_color color1 color2 step start end\n");
		return 0;
	}

	pr_info("%s: Testing commands\n", __func__);
	/* prepare lcd */
	mrun_command_list("lcd_power_on; lcd_set_base 1");

	for (i = start, cnt = 0; i <= end; i += step, cnt++) {
		printf ("\nLCD Power: (%d). 0x%x/0x%x --> %d/%d\n", cnt, color1, color2, i, total);
		ret = do_lcd_color_cmd(i, color1, color2);
		if (ret) {
			printf ("LCD Power: INTERRUPT\n");
			goto out;
		}
	}

	if (start <= end && ((end - start) % step)) {
		printf ("\nLCD Power: (%d). 0x%x/0x%x --> %d/%d\n", cnt, color1, color2, end, total);
		do_lcd_color_cmd(end, color1, color2);
	}

out:
	/* restore lcd */
	mrun_command_list("lcd_power_off; lcd_set_base 0");

	pr_info("%s: Everything went swimmingly\n", __func__);
	return 0;
}

U_BOOT_CMD(
	power_test_lcd_color,	10,	10,	cmd_power_test_lcd_color,
	"test Power Consumption of LCD in different color duty ratio",
	"power_test_lcd_color color1 color2 step start end"
);
