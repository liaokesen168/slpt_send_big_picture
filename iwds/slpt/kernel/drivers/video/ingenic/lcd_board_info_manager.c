#include <config.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <common.h>
#include <slpt.h>
#include <jzfb.h>
#include <malloc.h>
#include <slpt_app.h>
#include <linux/pr_info.h>
#include <linux/lcd_board_info.h>

#include "./jz_mipi_dsi/jz_mipi_dsih_hal.h"
#include "./jz_mipi_dsi/jz_mipi_dsi_regs.h"

#ifndef NULL
#define NULL 0
#endif

void flush_cache(ulong start_addr, ulong size);

void gpio_desc_print(gpio_desc_t *gpio_desc, const char *label) {
	lcd_info(label);
	lcd_info("\n");
	lcd_info_dec("gpio - ", gpio_desc->gpio);
	lcd_info_dec("valid - ", gpio_desc->valid_level);
	lcd_info("\n");
}

void regulator_desc_print(regulator_desc_t *reg_desc, const char *label) {
	lcd_info(label);
	lcd_info("\n");
	lcd_info_tag("regulator - ", reg_desc->id);
	lcd_info_dec("voltage - ", reg_desc->voltage);
	if (gpio_desc_valid(&reg_desc->gpio)) {
		lcd_info_dec("enable pin - ", reg_desc->gpio.gpio);
		lcd_info_dec("valid - ", reg_desc->gpio.valid_level);
		lcd_info("\n");
	} else {
		lcd_info("No enable pin\n");
	}
	lcd_info("\n");
}

struct lcd_bm_data {
	struct lcd_config_info lcd_config_info;
	struct jzfb jzfb;
	struct slpt_app_res *slpt_res_logo;

	unsigned int inited:1;
	unsigned int power:1;
	unsigned int config_inited:1;
	unsigned int lcd_inited:1;
	unsigned int fb_inited:1;
};

struct lcd_bm_data mlcd_bm_data = {
	.inited = 0,
	.power = 0,
	.config_inited = 0,
	.lcd_inited = 0,
	.fb_inited = 0,
};

static unsigned int get_lcd_bm_data_power(struct lcd_bm_data *lcd_bm_data)
{
	return lcd_bm_data->power;
}

static int lcd_bm_update_lcd_mode(struct lcd_bm_data *lcd_bm_data, int update) {
	struct lcd_config_info *lcd_config_info = &lcd_bm_data->lcd_config_info;
	struct jzfb_platform_data *lcd_pdata = (struct jzfb_platform_data *)lcd_config_info->lcd_pdata;
	int ret = 0;
	int flag = get_lcd_bm_data_power(&mlcd_bm_data); /* store the power state */

	if(!flag) {
		board_lcd_power_on();
		udelay(100); /* if don't delay, it will error. */
	}

	if (lcd_pdata->lcd.update_mode)
		ret = lcd_pdata->lcd.update_mode(update);
	else
		lcd_info_tag("lcd board info no update mode\n", __FUNCTION__);

	if(!flag) { /* we should return to the power state before we had changed */
		board_lcd_power_off();
	}

	return ret;
}

static int lcd_bm_power_on_lcd(struct lcd_bm_data *lcd_bm_data, int on) {
	struct lcd_config_info *lcd_config_info = &lcd_bm_data->lcd_config_info;
	struct jzfb_platform_data *lcd_pdata = (struct jzfb_platform_data *)lcd_config_info->lcd_pdata;

	if (lcd_pdata->lcd.power_on) {
		return lcd_pdata->lcd.power_on(on);
	}
	lcd_info_tag("lcd board info no power_on\n", __FUNCTION__);

	return 0;
}

static int lcd_bm_init_lcd_data(struct lcd_bm_data *lcd_bm_data) {
	struct lcd_config_info *lcd_config_info = &lcd_bm_data->lcd_config_info;
	struct jzfb_platform_data *lcd_pdata = (struct jzfb_platform_data *)lcd_config_info->lcd_pdata;
	int ret = 0;

	if (!lcd_bm_data->lcd_inited) {
		if (lcd_pdata->lcd.resource_init)
			ret = lcd_pdata->lcd.resource_init();
		else
			lcd_info_tag("lcd board info no resource init\n", __FUNCTION__);
		lcd_bm_data->lcd_inited = !ret;
	}

	return ret;
}

static void lcd_bm_get_data_table(struct lcd_bm_data *lcd_bm_data) {
	struct lcd_config_info *lcd_config_info = &lcd_bm_data->lcd_config_info;
	struct jzfb_platform_data*lcd_pdata = (struct jzfb_platform_data *)lcd_config_info->lcd_pdata;

	lcd_config_info->length_data_table = lcd_pdata->smart_config.length_data_table;
	lcd_config_info->data_table = lcd_pdata->smart_config.data_table;
}

static void *lcd_bm_get_lcd_platform_data(struct lcd_bm_data* lcd_bm_data) {
	return (void *) get_jzfb_platform_data(lcd_bm_data->lcd_config_info.lcd_board_info->lcd.name);
}

static int lcd_bm_gen_slcd_mode(struct lcd_bm_data *lcd_bm_data) {
	struct lcd_config_info *lcd_config_info = &lcd_bm_data->lcd_config_info;
	struct slcd_mode *slcd_mode = &lcd_config_info->slcd_mode;
	struct jzfb_platform_data *lcd_pdata = (struct jzfb_platform_data *)lcd_config_info->lcd_pdata;

	slcd_mode->cs = lcd_config_info->lcd_board_info->io_mcu.cs;
	slcd_mode->rd = lcd_config_info->lcd_board_info->io_mcu.rd;
	slcd_mode->bus_width = lcd_pdata->smart_config.bus_width;
	slcd_mode->csply_valid_high = lcd_pdata->smart_config.chip_select_valid_high;
	slcd_mode->rsply_cmd_high = lcd_pdata->smart_config.rsply_cmd_high;
	slcd_mode->wrply_active_high = lcd_pdata->smart_config.csply_active_high;
	slcd_mode->rdply_active_high = lcd_pdata->smart_config.rdply_active_high;

	lcd_pdata->slcd_mode = slcd_mode;

	return 0;
}

int init_lcd_config_info(struct lcd_bm_data *lcd_bm_data, struct lcd_board_info *lcd_board_info) {
	int ret = 0;
	struct lcd_config_info *lcd_config_info = &lcd_bm_data->lcd_config_info;

	if (lcd_bm_data->config_inited)
		goto return_ret;

	if (!lcd_board_info) {
		lcd_info_tag( "Failed to get lcd board info\n", __FUNCTION__);
		ret = -EINVAL;
		goto error_get_lcd_board_info_failed;
	}
	lcd_config_info->lcd_board_info = lcd_board_info;

	lcd_config_info->lcd_pdata = lcd_bm_get_lcd_platform_data(lcd_bm_data);
	if (!lcd_config_info->lcd_pdata) {
		lcd_info_tag( "Failed to get lcd platform data", __FUNCTION__);
		ret = -EINVAL;
		goto error_get_jzfb_config_info_failed;
	}

	if (lcd_bm_gen_slcd_mode(lcd_bm_data)) {
		lcd_info_tag( "Failed to gen slcd mode\n", __FUNCTION__);
		ret = -EINVAL;
		goto error_gen_slcd_mode_failed;
	}

	lcd_bm_get_data_table(lcd_bm_data);

	lcd_bm_data->config_inited = 1;
	goto return_ret;
error_gen_slcd_mode_failed:
error_get_jzfb_config_info_failed:
error_get_lcd_board_info_failed:
return_ret:
	return ret;
}

static int lcd_bm_init_fb_data(struct lcd_bm_data *lcd_bm_data) {
	int ret = 0;

	if (!lcd_bm_data->fb_inited) {
		ret = jzfb_init_data(&mlcd_bm_data.jzfb, mlcd_bm_data.lcd_config_info.lcd_pdata);
		lcd_bm_data->fb_inited = !ret;
	}

	return ret;
}

struct lcd_config_info *get_lcd_config_info(const char *name) {
	return &mlcd_bm_data.lcd_config_info;
}

static void lcd_bm_get_kernel_lcd_name(struct lcd_board_info *lcd_board_info) {
	const char *lcd_name = slpt_kernel_get_default_lcd(NULL);

	if (!lcd_board_info || !lcd_name)
		return;

	strcpy(lcd_board_info->lcd.name, lcd_name);
}

int lcd_bm_init_data(void) {
	int ret;

	lcd_info("Board lcd init\n");

	if (!mlcd_bm_data.inited) {
		lcd_bm_get_kernel_lcd_name(&lcd_board_info);

		ret = init_lcd_config_info(&mlcd_bm_data, &lcd_board_info);
		if (ret) {
			lcd_info_tag("Failed to init lcd config info\n", __FUNCTION__);
			goto error_lcd_info_init_failed;
		}

		ret = lcd_bm_init_lcd_data(&mlcd_bm_data);
		if (ret) {
			lcd_info_tag("Failed to init lcd resouce\n", __FUNCTION__);
			goto error_lcd_resource_init_failed;
		}

		ret = lcd_bm_init_fb_data(&mlcd_bm_data);
		if (ret) {
			lcd_info_tag("Failed to init jzfb data\n", __FUNCTION__);
			goto error_jzfb_init_data_failed;
		}

		/* lcd_bm_init_logo(&mlcd_bm_data); */

		mlcd_bm_data.inited = 1;
	}

	return 0;
error_jzfb_init_data_failed:
error_lcd_resource_init_failed:
error_lcd_info_init_failed:
	return ret;
}

#if 0
int lcd_bm_init_one_time(void) {
	struct slpt_app_res *res;
	struct slpt_app_res res_t = {
		.name = "logo-mem",
		.type = SLPT_RES_MEM,
		.addr = logo_colors,
		.length = sizeof(*logo_colors) + color_mode_size(logo_colors),
	};

	mlcd_bm_data.slpt_res_logo = NULL;

	res = slpt_kernel_register_app_res(&res_t, uboot_slpt_task);
	if (!res) {
		pr_info("SLPT: error: failed to register res logo-mem\n");
		return -ENOMEM;
	}

	mlcd_bm_data.slpt_res_logo = res;

	return 0;
}

SLPT_ARCH_INIT_ONETIME(lcd_bm_init_one_time);

int lcd_bm_exit_one_time(void) {
	if (!mlcd_bm_data.slpt_res_logo) {
		slpt_kernel_unregister_app_res(mlcd_bm_data.slpt_res_logo, uboot_slpt_task);
	}

	return 0;
}

SLPT_ARCH_EXIT_ONETIME(lcd_bm_exit_one_time);
#endif

struct dsi_device *get_default_dsi(void) {
	if (mlcd_bm_data.fb_inited) {
		return &mlcd_bm_data.jzfb.dsi;
	}

	pr_err("error: fb not been inited, init it first\n");
	return NULL;
}

struct fb_struct* get_default_fb(void) {
	if (mlcd_bm_data.fb_inited) {
		return &mlcd_bm_data.jzfb.fbs;
	}

	pr_err("error: fb not been inited, init it first\n");
	return NULL;
}

int lcd_pan_display(struct fb_struct *fbs, unsigned int offset) {
	struct jzfb *jzfb;

	if (!fbs) {
		pr_err("error: fbs can't be none \n");
		return -EINVAL;
	}

	jzfb = container_of(fbs, struct jzfb, fbs);

	return jzfb_pan_dispaly(jzfb, offset);
}

void lcd_wait_display_end(struct fb_struct *fbs) {
	struct jzfb *jzfb;

	if (!fbs) {
		pr_err("error: fbs can't be none \n");
		return;
	}

	jzfb = container_of(fbs, struct jzfb, fbs);

	return jzfb_lcdc_wait_dma_end(jzfb);
}

int lcd_flush_fb(struct fb_struct *fbs, unsigned int offset) {
	struct jzfb *jzfb;

	if (!fbs) {
		pr_err("error: fbs can't be none \n");
		return -EINVAL;
	}

	jzfb = container_of(fbs, struct jzfb, fbs);

	return jzfb_flush_fb(jzfb, offset);
}

int board_lcd_power_on(void) {

	int ret = -ENODEV;

	if (!slpt_kernel_get_fb_on())
		return 0;

	if (mlcd_bm_data.inited & !mlcd_bm_data.power) {
		lcd_bm_power_on_lcd(&mlcd_bm_data, 1);
		jzfb_enable(&mlcd_bm_data.jzfb);
		if (&mlcd_bm_data.jzfb.fbs)
			fbs_set_lcd_mem_base(&mlcd_bm_data.jzfb.fbs);
		ret = 0;
		mlcd_bm_data.power = 1;
	} else {
		lcd_info_tag("Please init lcd board info first before power on lcd\n", __FUNCTION__);
	}

	return ret;
}

int board_lcd_power_off(void) {

	int ret = -ENODEV;

	if (!slpt_kernel_get_fb_on())
		return 0;

	if (mlcd_bm_data.inited & mlcd_bm_data.power) {
		jzfb_disable(&mlcd_bm_data.jzfb);
		lcd_bm_power_on_lcd(&mlcd_bm_data, 0);
		ret = 0;
		mlcd_bm_data.power = 0;
	} else {
		lcd_info_tag("Please init lcd board info first before power off lcd\n", __FUNCTION__);
	}

	return ret;
}

static int cmd_lcd_color(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	unsigned int color1, color2, hor, w;
	struct fb_struct *fbs = get_default_fb();

	if (!fbs) {
		pr_err("lcd_color: failed to get default fbs\n");
		pr_err("lcd_color: try lcd_power_on and lcd_set_base 1\n");
		return -ENODEV;
	}

	if (argc <= 4) {
		hor = 1;
		w = fbs->xres / 3;
		color1 = 0x000000;
		color2 = 0xffffff;
	} else {
		if (!strcmp(argv[1], "hor"))
			hor = 1;
		else
			hor = 0;
		w = simple_strtoul(argv[2], NULL, 10);
		color1 = simple_strtoul(argv[3], NULL, 16);
		color2 = simple_strtoul(argv[4], NULL, 16);
	}

	pr_info("hor or ver : %s\n", hor ? "hor" : "ver");
	pr_info("w: %d\n", w);
	pr_info("color1: 0x%x\n", color1);
	pr_info("color2: 0x%x\n", color2);

	fbs_set_lcd_mem_base(&mlcd_bm_data.jzfb.fbs);

	if (hor)
		fb_clear_hor(w, color1, color2);
	else
		fb_clear_ver(w, color1, color2);

	jzfb_pan_dispaly(&mlcd_bm_data.jzfb, 0);

	return 0;
}

static int cmd_lcd_color_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	unsigned int color1, color2, hor, start, end, delay;
	int i;
	struct fb_struct *fbs = get_default_fb();

	if (!fbs) {
		pr_err("lcd_color: failed to get default fbs\n");
		pr_err("lcd_color: try lcd_power_on and lcd_set_base 1\n");
		return -ENODEV;
	}

	if (argc < 7) {
		hor = 1;
		start = fbs->xres / 3;
		end = fbs->xres - fbs->xres / 3;
		delay = 20;
		color1 = 0x000000;
		color2 = 0xffffff;
	} else {
		if (!strcmp(argv[1], "hor"))
			hor = 1;
		else
			hor = 0;
		start = simple_strtoul(argv[2], NULL, 10);
		end = simple_strtoul(argv[3], NULL, 10);
		delay = simple_strtoul(argv[4], NULL, 10);
		color1 = simple_strtoul(argv[5], NULL, 16);
		color2 = simple_strtoul(argv[6], NULL, 16);
	}

	pr_info("hor or ver : %s\n", hor ? "hor" : "ver");
	pr_info("start: %d\n", start);
	pr_info("end: %d\n", end);
	pr_info("delay: %d\n", delay);
	pr_info("color1: 0x%x\n", color1);
	pr_info("color2: 0x%x\n", color2);

	fbs_set_lcd_mem_base(&mlcd_bm_data.jzfb.fbs);

	for (i = start; i < end; ++i) {
		if (hor)
			fb_clear_hor(i, color1, color2);
		else
			fb_clear_ver(i, color1, color2);

		jzfb_pan_dispaly(&mlcd_bm_data.jzfb, 0);

		mdelay(delay);
	}

	return 0;
}

static int cmd_lcd_draw(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	int x0, y0, x1, y1, color;

	if (argc < 6) {
		pr_err("usage: lcd_draw x0 y0 x1 y1 color\n");
		return -EINVAL;
	}

	x0 = simple_strtoul(argv[1], NULL, 10);
	y0 = simple_strtoul(argv[2], NULL, 10);
	x1 = simple_strtoul(argv[3], NULL, 10);
	y1 = simple_strtoul(argv[4], NULL, 10);
	color = simple_strtoul(argv[5], NULL, 16);

	pr_info("draw color:%x line: (%d, %d) --- (%d, %d)\n", color, x0, y0, x1, y1);

	fbs_set_lcd_mem_base(&mlcd_bm_data.jzfb.fbs);

	lcd_one_line(x0, y0, x1, y1, color);

	jzfb_pan_dispaly(&mlcd_bm_data.jzfb, 0);

	return 0;
}

static int cmd_lcd_draw_line(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	int x0, y0, x1, y1, color, thickness;

	if (argc < 7) {
		pr_err("usage: lcd_draw x0 y0 x1 y1 thickness color\n");
		return -EINVAL;
	}

	x0 = simple_strtoul(argv[1], NULL, 10);
	y0 = simple_strtoul(argv[2], NULL, 10);
	x1 = simple_strtoul(argv[3], NULL, 10);
	y1 = simple_strtoul(argv[4], NULL, 10);
	thickness = simple_strtoul(argv[5], NULL, 10);
	color = simple_strtoul(argv[6], NULL, 16);

	pr_info("draw color:%x line: (%d, %d) --- (%d, %d) th:%d\n", color, x0, y0, x1, y1, thickness);

	fbs_set_lcd_mem_base(&mlcd_bm_data.jzfb.fbs);

	lcd_draw_line(x0, y0, x1, y1, thickness, color);

	jzfb_pan_dispaly(&mlcd_bm_data.jzfb, 0);

	return 0;
}

static int cmd_lcd_dma(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	pr_info("start dma called\n");
	return 0;
}

static int cmd_lcd_gpio(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	int value;

	if (argc < 2) {
		pr_err("usage: lcd_draw x0 y0 x1 y1 thickness color\n");
		return -EINVAL;
	}

	value = simple_strtoul(argv[1], NULL, 10);

	if (value == 1) {
		pr_info("lcd to function\n");
		slcd_pin_as_function(&mlcd_bm_data.lcd_config_info.slcd_mode);
	} else if (value == 0) {
		pr_info("lcd to gpio\n");
		slcd_data_as_output(&mlcd_bm_data.lcd_config_info.slcd_mode);
	} else if (value == 2) {
		pr_info("lcd to output low\n");
		slcd_pin_as_output_low(&mlcd_bm_data.lcd_config_info.slcd_mode);
	} else if (value == 3) {
		pr_info("lcd to input nopull\n");
		slcd_pin_as_input_nopull(&mlcd_bm_data.lcd_config_info.slcd_mode);
	}

	return 0;
}

void print_lcdc_state(struct jzfb *jzfb);

static int cmd_lcd_state(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {

	print_lcdc_state(&mlcd_bm_data.jzfb);

	return 0;
}

static int cmd_lcd_gpio_flush(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	int count, i;

	if (argc <= 1) {
		count = 100;
	} else {
		count = simple_strtoul(argv[1], NULL, 10);
	}
	pr_info("count to %d\n", count);

	for (i =0; i < 100; ++i) {
		jzfb_pan_dispaly(&mlcd_bm_data.jzfb, 0);
	}

	return 0;
}

static int cmd_lcd_fb_enable(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	int enable;

	if (argc <= 1) {
		enable = 1;
	} else {
		enable = simple_strtoul(argv[1], NULL, 10);
	}
	pr_info("enable %d\n", enable);

	if (enable) {
		jzfb_enable(&mlcd_bm_data.jzfb);
	} else {
		jzfb_disable(&mlcd_bm_data.jzfb);
	}

	return 0;
}

static int cmd_lcd_clk_enable(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	int enable;

	if (argc <= 1) {
		enable = 1;
	} else {
		enable = simple_strtoul(argv[1], NULL, 10);
	}
	pr_info("enable %d\n", enable);

	if (enable) {
		jzfb_clk_enable(&mlcd_bm_data.jzfb);
	} else {
		jzfb_clk_disable(&mlcd_bm_data.jzfb);
	}

	return 0;
}

static int cmd_lcd_set_base(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	int base;

	if (argc <= 1) {
		base = 1;
	} else {
		base = simple_strtoul(argv[1], NULL, 10);
	}
	pr_info("base %d\n", base);

	if (base) {
		jzfb_replace_kernel(&mlcd_bm_data.jzfb);
	} else {
		jzfb_restore_kernel(&mlcd_bm_data.jzfb);
	}

	return 0;
}

static int cmd_lcd_dma_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	int count;

	if (argc <= 1) {
		count = 100;
	} else {
		count = simple_strtoul(argv[1], NULL, 10);
	}
	pr_info("count %d\n", count);

	jzfb_dma_transfer(&mlcd_bm_data.jzfb, count);

	return 0;
}

static int cmd_lcd_flush_cache(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {

	jzfb_flush_cache(&mlcd_bm_data.jzfb);

	return 0;
}

static int cmd_lcd_write_data(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	struct slcd_mode *slcd = mlcd_bm_data.jzfb.slcd_mode;
	unsigned int cmd, data, i;

	if (argc <= 1) {
		pr_info("usage: lcd_write_data cmd data1 data2 ...\n");
		return -EINVAL;
	}

	cmd = simple_strtoul(argv[1], NULL, 16);
	slcd_write_cmd(slcd, cmd);
	pr_info("cmd: %x\n", cmd);
	for (i = 2; i < argc; ++i) {
		data = simple_strtoul(argv[i], NULL, 16);
		pr_info("data: %x\n", data);
	}

	return 0;
}

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))
#endif

struct smart_lcd_data_table *find_lcd_data_cmd(unsigned int cmd) {
	unsigned int i = 0;
	unsigned int len = mlcd_bm_data.lcd_config_info.length_data_table;
	struct smart_lcd_data_table *table = mlcd_bm_data.lcd_config_info.data_table;

	pr_info("data table: length: %u\n", len);

	for (i = 0; i < len; ++i) {
		if (table[i].type != 1)
			continue;
		if (table[i].value == cmd)
			return &table[i];
	}
	return NULL;
}

int lcd_change_init_data(unsigned int *data, unsigned int len) {
	struct smart_lcd_data_table *table = find_lcd_data_cmd(data[0]);
	int i;

	if (!table) {
		pr_err("lcd data: no table(cmd:%x) find\n", data[0]);
		return -ENODEV;
	}

	for (i = 1; i < len; ++i) {
		if (table[i].type != 2) {
			pr_err("lcd data: replace the error data table \n");
			return -ENODEV;
		}
		table[i].value = data[i];
	}
	return 0;
}


int lcd_change_init_data_cmd(unsigned int *data, unsigned int len) {
	struct smart_lcd_data_table *table = find_lcd_data_cmd(data[0]);
	int i;

	if (!table) {
		pr_err("lcd data: no table(cmd:%x) find\n", data[0]);
		return -ENODEV;
	}

	for (i = 0; i < len; ++i) {
		if ((i >= 1) && (table[i].type != 2)) {
			pr_err("lcd data: replace the error data table \n");
			return -ENODEV;
		}
		table[i].value = data[i + 1];
	}
	return 0;
}

static int lcd_init_data(void) {
	unsigned int i = 0;
	unsigned int len = mlcd_bm_data.lcd_config_info.length_data_table;
	struct smart_lcd_data_table *table = mlcd_bm_data.lcd_config_info.data_table;
	struct slcd_mode *slcd = &mlcd_bm_data.lcd_config_info.slcd_mode;

	slcd_data_as_output(slcd);

	for (i = 0; i < len; ++i) {
		if (table[i].type == 1)
			slcd_write_cmd(slcd, table[i].value);
		else if (table[i].type == 2)
			slcd_write_data(slcd, table[i].value);
		if (table[i].udelay)
			udelay(table[i].udelay);

		pr_info("%s : %x\n", table[i].type == 1 ? "\ncmd" : "dat", table[i].value);
	}
	return 0;
}

static int cmd_lcd_replace_data(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	unsigned int data[10], i;

	if (argc <= 1 || argc >= 11 ) {
		pr_info("usage: lcd_replace cmd data1 data2 ...(max 10)\n");
		return -EINVAL;
	}

	data[0] = simple_strtoul(argv[1], NULL, 16);
	pr_info("cmd: %x\n", data[0]);
	for (i = 2; i < argc; ++i) {
		data[i - 1] = simple_strtoul(argv[i], NULL, 16);
		pr_info("data: %x\n", data[i - 1]);
	}
	pr_info("len: %d\n", i - 1);

	lcd_change_init_data(data, i - 1);

	return 0;
}

static int cmd_lcd_replace_data_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	unsigned int data[10], i;

	if (argc <= 2 || argc >= 11 ) {
		pr_info("usage: lcd_replace_ cmd cmd data1 data2 ...(max 9)\n");
		return -EINVAL;
	}

	data[0] = simple_strtoul(argv[1], NULL, 16);
	pr_info("cmd: %x\n", data[0]);
	data[1] = simple_strtoul(argv[2], NULL, 16);
	pr_info("cmd: %x\n", data[2]);

	for (i = 3; i < argc; ++i) {
		data[i - 1] = simple_strtoul(argv[i], NULL, 16);
		pr_info("data: %x\n", data[i - 1]);
	}
	pr_info("len: %d\n", i - 1);

	lcd_change_init_data_cmd(data, i - 1);

	return 0;
}

static int cmd_lcd_init_data(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {

	pr_info("lcd init data\n");
	lcd_init_data();
	return 0;
}

#if 0
static int cmd_lcd_logo(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	write_colors_to_fb(&mlcd_bm_data.jzfb.fbs, logo_colors);
	lcd_pan_display(&mlcd_bm_data.jzfb.fbs, 0);

	return 0;
}

U_BOOT_CMD(
	lcd_logo, 10, 1, cmd_lcd_logo,
	"lcd show logo in position",
	"lcd show logo in position"
);

static int cmd_lcd_logo_pos(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	struct position pos;

	if (argc < 3) {
		pos.x = 0;
		pos.y = 0;
	} else {
		pos.x = simple_strtol(argv[1], NULL, 10);
		pos.y = simple_strtol(argv[2], NULL, 10);
	}

	pr_info("x,y : %d,%d\n", pos.x, pos.y);

	write_colors_to_fb_position(&mlcd_bm_data.jzfb.fbs, &pos, logo_colors);
	lcd_pan_display(&mlcd_bm_data.jzfb.fbs, 0);

	return 0;
}

U_BOOT_CMD(
	lcd_logo_pos, 10, 1, cmd_lcd_logo_pos,
	"lcd show logo",
	"lcd show logo"
);

#endif
U_BOOT_CMD(
	lcd_init_data, 10, 1, cmd_lcd_init_data,
	"lcd init data",
	"lcd init data"
);

U_BOOT_CMD(
	lcd_replace_data_cmd, 10, 1, cmd_lcd_replace_data_cmd,
	"lcd replace cmd and data",
	"lcd replace cmd and data"
);

U_BOOT_CMD(
	lcd_replace_data, 10, 1, cmd_lcd_replace_data,
	"lcd replace data",
	"lcd replace data"
);

U_BOOT_CMD(
	lcd_write_data, 10, 1, cmd_lcd_write_data,
	"lcd write cmd and data",
	"lcd write cmd and data"
);

U_BOOT_CMD(
	lcd_flush_cache, 10, 1, cmd_lcd_flush_cache,
	"lcd flush fb cache",
	"lcd flush fb cache"
);

U_BOOT_CMD(
	lcd_dma_test, 10, 1, cmd_lcd_dma_test,
	"lcd test dma speed",
	"lcd test dma speed"
);

U_BOOT_CMD(
	lcd_set_base, 10, 1, cmd_lcd_set_base,
	"lcd set uboot fb base",
	"lcd set uboot fb base"
);

U_BOOT_CMD(
	lcd_clk_enable, 10, 1, cmd_lcd_clk_enable,
	"lcd clk enable",
	"lcd clk enable"
);

U_BOOT_CMD(
	lcd_fb_enable, 10, 1, cmd_lcd_fb_enable,
	"lcd enable fb, jzfb",
	"lcd enable fb, jzfb"
);

U_BOOT_CMD(
	lcd_gpio_flush, 10, 1, cmd_lcd_gpio_flush,
	"lcd gpio flush",
	"lcd gpio flush"
);

U_BOOT_CMD(
	lcd_state, 10, 1, cmd_lcd_state,
	"lcd print lcdc state",
	"lcd print lcdc state"
);

U_BOOT_CMD(
	lcd_gpio, 10, 1, cmd_lcd_gpio,
	"lcd gpio mode",
	"lcd gpio mode"
);

U_BOOT_CMD(
	lcd_dma, 10, 1, cmd_lcd_dma,
	"draw line to lcd",
	"draw line to lcd"
);

U_BOOT_CMD(
	lcd_draw_line, 10, 1, cmd_lcd_draw_line,
	"draw line to lcd",
	"draw line to lcd"
);

U_BOOT_CMD(
	lcd_draw, 6, 1, cmd_lcd_draw,
	"draw line to lcd",
	"draw line to lcd"
);

U_BOOT_CMD(
	lcd_color, 10, 1, cmd_lcd_color,
	"write color to lcd",
	"lcd_color hor/ver w color1 color2 "
	"e.g lcd_color hor 50 0xff0000 0x0000ff"
);

U_BOOT_CMD(
	lcd_color_test, 10, 1, cmd_lcd_color_test,
	"test case: write color to lcd",
	"lcd_color_test hor/ver start end delay color1 color2 "
	"e.g lcd_color_test hor 50 120 20 0xff0000 0x0000ff"
);

static int cmd_lcd_bm_init(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	return lcd_bm_init_data();
}

U_BOOT_CMD(
	lcd_bm_init_data, 6, 1, cmd_lcd_bm_init,
	"init lcd and fb data",
	"init lcd and fb data"
);

static int cmd_lcd_power_on(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	return board_lcd_power_on();
}

U_BOOT_CMD(
	lcd_power_on, 6, 1, cmd_lcd_power_on,
	"lcd power on",
	"power on the lcd on this board"
);

static int cmd_lcd_power_off(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	return board_lcd_power_off();
}

U_BOOT_CMD(
	lcd_power_off, 6, 1, cmd_lcd_power_off,
	"lcd power off",
	"power off the lcd on this board"
);

int lcd_bm_update_status(void) {
	if (!slpt_kernel_get_fb_on())
		return 0;
	lcd_bm_update_lcd_mode(&mlcd_bm_data, 1);
	return jzfb_update_status(&mlcd_bm_data.jzfb);
}
SLPT_ARCH_INIT_EVERYTIME(lcd_bm_update_status);

int lcd_bm_restore_status(void) {
	if (!slpt_kernel_get_fb_on())
		return 0;
	lcd_bm_update_lcd_mode(&mlcd_bm_data, 0);
	return jzfb_restore_status(&mlcd_bm_data.jzfb);
}
SLPT_ARCH_EXIT_EVERYTIME(lcd_bm_restore_status);
