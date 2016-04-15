#include <config.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <common.h>
#include <slpt.h>
#include <jzfb.h>
#include <malloc.h>
#include <linux/pr_info.h>
#include <linux/lcd_board_info.h>

struct truly240240_data {
	struct device *dev;
	struct lcd_config_info *lcd_config_info;
	struct slcd_mode *slcd_mode;
	unsigned int power:1;
	unsigned int inited:1;
};

extern struct smart_lcd_data_table truly240240_update_table[];
extern unsigned int truly240240_update_table_length;

extern struct smart_lcd_data_table truly240240_restore_table[];
extern unsigned int truly240240_restore_table_length;

int truly240240_resource_init(struct truly240240_data *truly240240 ,struct lcd_config_info *lcd_config_info) {
	int ret = 0;

	truly240240->lcd_config_info = lcd_config_info;
	truly240240->power = !lcd_config_info->lcd_board_info->lcd.need_init;
	truly240240->slcd_mode = &lcd_config_info->slcd_mode;
	truly240240->inited = 0;

	ret = slcd_request_gpio(truly240240->slcd_mode);
	if (ret) {
		lcd_info_tag("Failed to request slcd gpio", __FUNCTION__);
		ret = -EINVAL;
		goto error_slcd_gpio_request_failed;
	}

	slcd_init_interface(truly240240->slcd_mode);

	truly240240->inited = 1;

	goto return_ret;
error_slcd_gpio_request_failed:
return_ret:
	return ret;
}

struct truly240240_data mtruly240240 = {
	.inited = 0,
};

int truly240240_init(void) {
	pr_debug("truly240240: %s called\n", __FUNCTION__);
	return 0;
}

int truly240240_res_init(void) {
	struct lcd_config_info *lcd_config_info = get_lcd_config_info("truly240240");

	pr_debug("truly240240: %s called\n", __FUNCTION__);

	memset(&mtruly240240, 0, sizeof(mtruly240240));

	if (!lcd_config_info) {
		lcd_info("Failed to get lcd config info");
		return -1;
	}

	return truly240240_resource_init(&mtruly240240, lcd_config_info);
}

int truly240240_power_on(int on) {
	pr_debug("truly240240: %s called\n", __FUNCTION__);

	if (!mtruly240240.inited) {
		pr_info("truly240240: truly240240 must be init first\n");
		return -ENODEV;
	}

	if (on) {
		/* slcd_data_as_output(mtruly240240.slcd_mode); */
	} else {
		/* slcd_pin_as_function(mtruly240240.slcd_mode); */
	}

	mtruly240240.power = on;

	return 0;
}

int truly240240_update_mode(int update) {
	pr_debug("truly240240: %s called\n", __FUNCTION__);

	if (update) {
		slcd_write_datas(mtruly240240.slcd_mode, truly240240_update_table, truly240240_update_table_length);
	} else {
		slcd_write_datas(mtruly240240.slcd_mode, truly240240_restore_table, truly240240_restore_table_length);
	}
	return 0;
}
