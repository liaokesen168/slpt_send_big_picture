#include <config.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <common.h>
#include <slpt.h>
#include <jzfb.h>
#include <malloc.h>

#include <linux/lcd_board_info.h>

struct seps645b_data {
	struct device *dev;
	struct lcd_config_info *lcd_config_info;
	unsigned int pwm_level;
	gpio_desc_t *lcd_rst;
	regulator_desc_t *lcd_vio;
	regulator_desc_t *lcd_vcc;
	struct slcd_mode *slcd_mode;
	unsigned int power:1;
	unsigned int inited:1;
};

int seps645b_resource_init(struct seps645b_data *seps645b ,struct lcd_config_info *lcd_config_info) {
	int ret;
	struct slcd_mode *slcd = &lcd_config_info->slcd_mode;

	seps645b->lcd_config_info = lcd_config_info;
	seps645b->power = !lcd_config_info->lcd_board_info->lcd.need_init;
	seps645b->pwm_level = 0;
	seps645b->lcd_rst = &lcd_config_info->lcd_board_info->ctrl_signal.lcd_rst;
	seps645b->lcd_vio = &lcd_config_info->lcd_board_info->power_supply.lcd_vio;
	seps645b->lcd_vcc = &lcd_config_info->lcd_board_info->power_supply.lcd_vcc;
	seps645b->slcd_mode = &lcd_config_info->slcd_mode;
	seps645b->inited = 0;

	ret = slcd_request_gpio(slcd);
	if (ret) {
		lcd_info_tag("Failed to request slcd gpio", __FUNCTION__);
		ret = -EINVAL;
		goto error_slcd_gpio_request_failed;
	}

	ret = gpio_desc_request(seps645b->lcd_rst, "lcd-rst");
	if (ret) {
		lcd_info_tag("Failed to request lcd rst", __FUNCTION__);
		ret = -EINVAL;
		goto error_lcd_rst_gpio_request_failed;
	}

	ret = regulator_desc_get(seps645b->dev, seps645b->lcd_vio);
	if (ret) {
		lcd_info_tag("Failed to get lcd vio", __FUNCTION__);
		ret = -EINVAL;
		goto error_lcd_vio_get_failed;
	}

	ret = regulator_desc_get(seps645b->dev, seps645b->lcd_vcc);
	if (ret) {
		lcd_info_tag("Failed to get lcd vcc", __FUNCTION__);
		ret = -EINVAL;
		goto error_lcd_vcc_get_failed;
	}

#define SEPS645b_RES_DEBUG
#ifdef SEPS645b_RES_DEBUG
	lcd_info("SEPS645B resource debug\n");
	lcd_info("=======================\n");
	gpio_desc_print(seps645b->lcd_rst, "lcd-rst");
	regulator_desc_print(seps645b->lcd_vio, "lcd-vio");
	regulator_desc_print(seps645b->lcd_vcc, "lcd-vcc");
	lcd_info("=======================\n");
#endif

	seps645b->inited = 1;

	return 0;
error_lcd_vcc_get_failed:
	regulator_desc_put(seps645b->lcd_vio);
error_lcd_vio_get_failed:
	gpio_desc_free(seps645b->lcd_rst);
error_lcd_rst_gpio_request_failed:
	slcd_free_gpio(slcd);
error_slcd_gpio_request_failed:
	return ret;
}

 #define __lcd_set_backlight_level(x) do {} while (0)

int seps645b_power(struct seps645b_data *seps645b, int on) {
	if (!seps645b->inited) {
		lcd_info("seps645b must be inited\n");
		return -1;
	}

	if (on && !seps645b->power) {
		__lcd_set_backlight_level(0);

		regulator_desc_disable(seps645b->lcd_vcc);
		mdelay(500);

		slcd_init_interface(seps645b->slcd_mode);
		gpio_desc_direction_output_high(seps645b->lcd_rst);
		/* vio on */
		regulator_desc_enable(seps645b->lcd_vio);

		/* reset lcd */
		gpio_desc_set_low(seps645b->lcd_rst);
		mdelay(5);
		gpio_desc_set_high(seps645b->lcd_rst);

		/* wait stable */
		mdelay(100);

		/* vcc_c on */
		regulator_desc_enable(seps645b->lcd_vcc);
		lcd_info("vcc start on\n");
		mdelay(300);
		__lcd_set_backlight_level(30);
		mdelay(200);
		__lcd_set_backlight_level(80);
		mdelay(200);
		__lcd_set_backlight_level(150);
		mdelay(200);
		__lcd_set_backlight_level(245);
		mdelay(200);

		lcd_info("SEPS645b power on ok\n");
	} else if (!on && seps645b->power) {
		__lcd_set_backlight_level(0);
		regulator_desc_disable(seps645b->lcd_vcc);
		mdelay(100);
		regulator_desc_disable(seps645b->lcd_vio);
		mdelay(20);
		lcd_info("SEPS645b power off ok\n");
	}

	return 0;
}

static int seps645b_send_init_data(struct seps645b_data *seps645b) {
	int i, j;
	struct slcd_mode *mode = seps645b->slcd_mode;
	struct smart_lcd_data_table *data_table = seps645b->lcd_config_info->data_table;
	unsigned int length_data_table = seps645b->lcd_config_info->length_data_table;

	slcd_data_as_output(mode);
	j = 0;
	for (i=0; i < length_data_table; i++) {
		/* lcd_info_dec("line :", i); */
		/* j++; */
		/* if ( j <= 27 && j >= 25) {		/\* > 25, 31, 38,50*\/ */
		/* 	mdelay(1); */
		/* } */
		mdelay(1);
		if (data_table[i].type == 1)
			slcd_write_cmd(mode, data_table[i].value);
		else if (data_table[i].type == 2)
			slcd_write_data(mode, data_table[i].value);
		if (data_table[i].udelay)
			udelay(data_table[i].udelay);
	}

	mdelay(250);

	slcd_write_cmd(mode, 0x0c);

	for ( i = 0; i < 176; ++i) {
		for (j = 0; j < 132; ++j) {
			slcd_write_data(mode, 0x00);
			slcd_write_data(mode, 0x00);
		}
	}
	mdelay(3000);

#if 0

	lcd_info("seps645b gpio write color\n");
	for ( i = 0; i < 176; ++i) {
		for (j = 0; j < 132; ++j) {
			slcd_write_data(mode, 0xf8);
			slcd_write_data(mode, 0x00);
		}
	}
	mdelay (3000);
	for ( i = 0; i < 176; ++i) {
		for (j = 0; j < 132; ++j) {
			slcd_write_data(mode, 0x07);
			slcd_write_data(mode, 0xe0);
		}
	}
	mdelay (3000);
	for ( i = 0; i < 176; ++i) {
		for (j = 0; j < 132; ++j) {
			slcd_write_data(mode, 0x00);
			slcd_write_data(mode, 0x1f);
		}
	}

#endif

	lcd_info("lcd gpio send list ok\n");

	slcd_pin_as_function(mode);

	return 0;
}

struct seps645b_data mseps645b = {
	.inited = 0,
};

int seps645b_res_init(void) {
	struct seps645b_data *seps645b = &mseps645b;
	struct lcd_config_info *lcd_config_info = get_lcd_config_info("seps645b");

	memset(seps645b, 0, sizeof(*seps645b));

	if (!lcd_config_info) {
		lcd_info("Failed to get lcd config info");
		return -1;
	}

	return 0;

	return seps645b_resource_init(seps645b, lcd_config_info);
}

int seps645b_power_on(int on) {
	struct seps645b_data *seps645b = &mseps645b;

	return 0;
	if (!seps645b->inited) {
		lcd_info("ERROR: Seps645b need to init \n");
		return -1;
	}

	return seps645b_power(&mseps645b, on);
}

int seps645b_init(void)
{
	struct seps645b_data *seps645b = &mseps645b;

	return 0;
	return seps645b_send_init_data(seps645b);
}
