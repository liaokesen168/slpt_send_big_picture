#ifndef _LCD_BOARD_INFO_H_
#define _LCD_BOARD_INFO_H_

#include <linux/lcd_info_debug.h>
#include <linux/gpio_desc_t.h>
#include <linux/regulator_desc_t.h>
#include <linux/lcd_mcu.h>

struct lcd_board_info {
	struct {
#define MAX_LCD_NAME_LEN 30
		char name[MAX_LCD_NAME_LEN];
		int need_init;
	} lcd;

	struct {
		gpio_desc_t lcd_rst;
		gpio_desc_t lcd_disp;
		gpio_desc_t lcd_te;
	} ctrl_signal;

	/* lcd's power supply conditions:
	 *
	 *   1) vcc for analog, vdd for digital, vio for io
	 *   2) vcc for analog and digital, vio for io
	 *   3) vcc for analog , vdd for digital and io
	 *   4) vdd for vdd and analog, vio for io
	 *
	 */
	struct {
		regulator_desc_t lcd_vcc;	   /* analog circuit power supply */
		regulator_desc_t lcd_vdd;	   /* lcd digital circuit power supply  */
		regulator_desc_t lcd_vio;	   /* lcd io circuit power supply */
	} power_supply;

	/* just now for ingenic lcd controller, so except cs other is  */
	struct {
		int cs;
		int rd;
		int mode;
	} io_mcu;
};

struct lcd_config_info {
	struct lcd_board_info *lcd_board_info;
	void *lcd_pdata;
	struct slcd_mode slcd_mode;
	struct smart_lcd_data_table *data_table;
	unsigned int length_data_table;
};

struct jzfb_platform_data;

struct lcd_config_info *get_lcd_config_info(const char *name);

struct jzfb_platform_data *get_jzfb_platform_data(const char *name);

extern struct lcd_board_info lcd_board_info;

extern void gpio_desc_print(gpio_desc_t *gpio_desc, const char *label);

extern void regulator_desc_print(regulator_desc_t *reg_desc, const char *label);

#endif /* _LCD_BOARD_INFO_H_ */
