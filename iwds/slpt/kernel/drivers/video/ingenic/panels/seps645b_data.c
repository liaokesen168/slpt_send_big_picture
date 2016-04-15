/**
 * jzfb platform data : SEPS645B OLED
 *
 */
#include <config.h>
#include <jzfb.h>
#include <common.h>

#ifdef CONFIG_SLCD_SEPS645B

extern int seps645b_init(void);
extern int seps645b_res_init(void);
extern int seps645b_power_on(int on);

static struct smart_lcd_data_table seps645b_data_table[]= {

	/*Standb off */
	{0x03, 0x03, 1, 0},
	{0x03, 0x00, 2, 1000},

	/*Set Oscillator operation & frame rate */
	{0x04, 0x04, 1, 0},
	{0x04, 0x82, 2, 0},

	/*Set MCU Interface */
	{0x05, 0x05, 1, 0},
	{0x05, 0x01, 2, 0},

	/*Set Interface Mode */
	{0x20, 0x20, 1, 0},
	{0x20, 0x00, 2, 0},

	/*Set Discharge width */
	{0x15, 0x15, 1, 0},
	{0x15, 0x00, 2, 0},

	/*Set peak pulse width */
	{0x17, 0x17, 1, 0},
#if 0
	{0x17, 0x05, 2, 0},
	{0x17, 0x05, 2, 0},
	{0x17, 0x05, 2, 0},
#else
	{0x17, 0x00, 2, 0},
	{0x17, 0x00, 2, 0},
	{0x17, 0x00, 2, 0},
#endif

	/*Set peak pulse delay */
	{0x16, 0x16, 1, 0},
#if 0
	{0x16, 0x05, 2, 0},
#else
	{0x16, 0x00, 2, 0},
#endif

	/*Set peak current */
	{0x14, 0x14, 1, 0},
	{0x14, 0x00, 2, 0},
	{0x14, 0x00, 2, 0},
	{0x14, 0x00, 2, 0},

	/*Set driving current */
	{0x13, 0x13, 1, 0},
#if 0
	{0x13, 0xf0, 2, 0},
	{0x13, 0xf0, 2, 0},
	{0x13, 0x68, 2, 0},
#else
	{0x13, 0x7f, 2, 0},
	{0x13, 0xff, 2, 0},
	{0x13, 0xff, 2, 0},
#endif

	/*Set row scan operation */
	{0x18, 0x18, 1, 0},
	{0x18, 0x20, 2, 0},

	/*set internal regulator for row scan */
	{0x19, 0x19, 1, 0},
	{0x19, 0x10, 2, 0},

	/*Set GRAM Write direction */
	{0x06, 0x06, 1, 0},
	{0x06, 0x01, 2, 0},

#if 0
	/*Set active display size */
	{0x10, 0x10, 1, 0},
	{0x10, 0x2c, 2, 0},
	{0x10, 0xaf, 2, 0},
	{0x10, 0x00, 2, 0},
	{0x10, 0xaf, 2, 0},

	/*Set memory area(address) to write a display data */
	{0x07, 0x07, 1, 0},
	{0x07, 0x00, 2, 0},
	{0x07, 0x83, 2, 0},
	{0x07, 0x00, 2, 0},
	{0x07, 0xaf, 2, 0},

#else
	/*Set active display size */
	{0x10, 0x10, 1, 0},
	{0x10, 0x2c, 2, 0},
	{0x10, 0xaf, 2, 0},
	{0x10, 0x00, 2, 0},
	{0x10, 0xaf, 2, 0},

	/*Set memory area(address) to write a display data */
	{0x07, 0x07, 1, 0},
	{0x07, 0x00, 2, 0},
	{0x07, 0x83, 2, 0},
	{0x07, 0x00, 2, 0},
	{0x07, 0xaf, 2, 0},

#endif

	{0x12, 0x12, 1, 0},
	{0x12, 0x87, 2, 0},

	/*Frame Rate vd */
	{0x90, 0x90, 1, 0},
	{0x90, 0x06, 2, 0},

	{0x91, 0x91, 1, 0},
	{0x91, 0x01, 2, 0},

#if 1
	/*Set display start point */
	{0x11, 0x11, 1, 0},
	{0x11, 0x00, 2, 0},
	{0x11, 0x00, 2, 0},
#else
	/*Set display start point */
	{0x11, 0x11, 1, 0},
	{0x11, 0x00, 2, 0},
	{0x11, 0x0a, 2, 0},
#endif
	/*Display on */
	{0x02, 0x02, 1, 0},
	{0x02, 0x01, 2, 0},

	/* red draw */
	{0x0c, 0x0c, 1, 0},

};

static struct fb_videomode seps645b_jzfb_videomode = {
	.name = "132x176",
	.refresh = 60,
	.xres = 132,
	.yres = 176,
	.pixclock = KHZ2PICOS(4180 * 2),
	.left_margin = 0,
	.right_margin = 0,
	.upper_margin = 0,
	.lower_margin = 0,
	.hsync_len = 0,
	.vsync_len = 0,
	.sync = FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
};

unsigned long seps645b_gram_cmd[] = {
	0x0c0c0c0c,
};

struct jzfb_platform_data seps645b_jzfb_pdata = {
	.lcd.name = "seps645b",
	.lcd.resource_init = seps645b_res_init,
	.lcd.power_on = seps645b_power_on,

	.num_modes = 1,
	.modes = &seps645b_jzfb_videomode,

	.lcd_type = LCD_TYPE_LCM,
	.bpp = 18,
	.width = 25,
	.height = 34,
	.pinmd = 0,

	.pixclk_falling_edge = 0,
	.date_enable_active_low = 0,

	.alloc_vidmem = 1,
	.lvds = 0,

	.smart_config.smart_type = SMART_LCD_TYPE_PARALLEL,
	.smart_config.cmd_width = SMART_LCD_CWIDTH_8_BIT_ONCE,
	.smart_config.data_width = SMART_LCD_DWIDTH_8_BIT_ONCE_PARALLEL_SERIAL,
	.smart_config.data_width2 = SMART_LCD_DWIDTH_8_BIT_TWICE_TIME_PARALLEL,
	.smart_config.clkply_active_rising = 0,
	.smart_config.rsply_cmd_high = 0,
	.smart_config.csply_active_high = 0,
	.smart_config.rdply_active_high = 0,
	.smart_config.chip_select_valid_high = 0,
	/* write graphic ram command, in word, for example 8-bit bus, write_gram_cmd=C3C2C1C0. */
	.smart_config.write_gram_cmd = seps645b_gram_cmd,
	.smart_config.length_cmd = ARRAY_SIZE(seps645b_gram_cmd),
	.smart_config.bus_width = 8,
	.smart_config.length_data_table = ARRAY_SIZE(seps645b_data_table),
	.smart_config.data_table = seps645b_data_table,
	.smart_config.init = seps645b_init,

	.dither_enable = 1,
	.dither.dither_red = 1, /* 6bit */
	.dither.dither_red = 1, /* 6bit */
	.dither.dither_red = 1, /* 6bit */
};

#endif	/* CONFIG_SLCD_SEPS645B */
