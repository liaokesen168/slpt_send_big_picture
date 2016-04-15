#include <config.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <common.h>
#include <slpt.h>
#include <jzfb.h>
#include <malloc.h>
#include <linux/pr_info.h>
#include <linux/lcd_board_info.h>

struct truly320320_data {
	struct device *dev;
	struct lcd_config_info *lcd_config_info;
	struct slcd_mode *slcd_mode;
	unsigned int power:1;
	unsigned int inited:1;
};

extern struct smart_lcd_data_table truly320320_update_table[];
extern unsigned int truly320320_update_table_length;

extern struct smart_lcd_data_table truly320320_restore_table[];
extern unsigned int truly320320_restore_table_length;

int truly320320_resource_init(struct truly320320_data *truly320320 ,struct lcd_config_info *lcd_config_info) {
	int ret = 0;

	truly320320->lcd_config_info = lcd_config_info;
	truly320320->power = !lcd_config_info->lcd_board_info->lcd.need_init;
	truly320320->slcd_mode = &lcd_config_info->slcd_mode;
	truly320320->inited = 0;

	ret = slcd_request_gpio(truly320320->slcd_mode);
	if (ret) {
		lcd_info_tag("Failed to request slcd gpio", __FUNCTION__);
		ret = -EINVAL;
		goto error_slcd_gpio_request_failed;
	}

	slcd_init_interface(truly320320->slcd_mode);

	truly320320->inited = 1;

	goto return_ret;
error_slcd_gpio_request_failed:
return_ret:
	return ret;
}

struct truly320320_data mtruly320320 = {
	.inited = 0,
};

int truly320320_init(void) {
	pr_debug("truly320320: %s called\n", __FUNCTION__);
	return 0;
}

int truly320320_res_init(void) {
	struct lcd_config_info *lcd_config_info = get_lcd_config_info("truly320320");

	pr_debug("truly320320: %s called\n", __FUNCTION__);

	memset(&mtruly320320, 0, sizeof(mtruly320320));

	if (!lcd_config_info) {
		lcd_info("Failed to get lcd config info");
		return -1;
	}

	return truly320320_resource_init(&mtruly320320, lcd_config_info);
}

int truly320320_power_on(int on) {
	pr_debug("truly320320: %s called\n", __FUNCTION__);

	if (!mtruly320320.inited) {
		pr_info("truly320320: truly320320 must be init first\n");
		return -ENODEV;
	}

	if (on) {
		/* slcd_data_as_output(mtruly320320.slcd_mode); */
	} else {
		/* slcd_pin_as_function(mtruly320320.slcd_mode); */
	}

	mtruly320320.power = on;

	return 0;
}

int truly320320_update_mode(int update) {
	pr_info("truly320320: %s called, updata= %d\n", __FUNCTION__, update);

	if (update) {
		slcd_write_datas(mtruly320320.slcd_mode, truly320320_update_table, truly320320_update_table_length);
	} else {
		slcd_write_datas(mtruly320320.slcd_mode, truly320320_restore_table, truly320320_restore_table_length);
	}
	return 0;
}
