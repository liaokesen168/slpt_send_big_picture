/**
 * jzfb platform data : SEPS645B OLED
 *
 */
#include <config.h>
#include <jzfb.h>
#include <common.h>

extern int truly320320_init(void);
extern int truly320320_res_init(void);
extern int truly320320_power_on(int on);
extern int truly320320_update_mode(int update);

#define CONFIG_TRULY_240X240_ROTATE_180
/* #define CONFIG_SLCD_TRULY_18BIT */

static struct smart_lcd_data_table truly320320_data_table[] = {
	/* LCD init code */
	{0x01, 0x01, 1, 120000},  //soft reset, 120 ms = 120 000 us
	{0x11, 0x11, 1, 5000},	  /* sleep out 5 ms  */

	{0x36, 0x36, 1, 0},
#ifdef	CONFIG_TRULY_240X240_ROTATE_180
	/*{0x36, 0xc0, 2, 0}, //40*/
	{0x36, 0xd0, 2, 0}, //40
#else
	{0x36, 0x00, 2, 0}, //40
#endif

	{0x2a, 0x2a, 1, 0},
	{0x2a, 0x00, 2, 0},
	{0x2a, 0x00, 2, 0},
	{0x2a, 0x00, 2, 0},
	{0x2a, 0xef, 2, 0},

	{0x2b, 0x2b, 1, 0},
	{0x2b, 0x00, 2, 0},
	{0x2b, 0x00, 2, 0},
	{0x2b, 0x00, 2, 0},
	{0x2b, 0xef, 2, 0},

	{0x3a, 0x3a, 1, 0},
#if defined(CONFIG_SLCD_TRULY_18BIT)  //if 18bit/pixel unusual. try to use 16bit/pixel
	{0x3a, 0x06, 2, 0}, //55   m
#else
	{0x3a, 0x05, 2, 0}, //55   m
#endif
//	{0x3a, 0x55, 2, 0},

	{0xb2, 0xb2, 1, 0},
	{0xb2, 0x7f, 2, 0},
	{0xb2, 0x7f, 2, 0},
	{0xb2, 0x01, 2, 0},
	{0xb2, 0xde, 2, 0},
	{0xb2, 0x33, 2, 0},

	{0xb3, 0xb3, 1, 0},
	{0xb3, 0x10, 2, 0},
	{0xb3, 0x05, 2, 0},
	{0xb3, 0x0f, 2, 0},

	{0xb4, 0xb4, 1, 0},
	{0xb4, 0x0b, 2, 0},

	{0xb7, 0xb7, 1, 0},
	{0xb7, 0x35, 2, 0},

	{0xbb, 0xbb, 1, 0},
	{0xbb, 0x28, 2, 0}, //23

	{0xbc, 0xbc, 1, 0},
	{0xbc, 0xec, 2, 0},

	{0xc0, 0xc0, 1, 0},
	{0xc0, 0x2c, 2, 0},

	{0xc2, 0xc2, 1, 0},
	{0xc2, 0x01, 2, 0},

	{0xc3, 0xc3, 1, 0},
	{0xc3, 0x1e, 2, 0}, //14

	{0xc4, 0xc4, 1, 0},
	{0xc4, 0x20, 2, 0},

	{0xc6, 0xc6, 1, 0},
	{0xc6, 0x14, 2, 0},

	{0xd0, 0xd0, 1, 0},
	{0xd0, 0xa4, 2, 0},
	{0xd0, 0xa1, 2, 0},

	{0xe0, 0xe0, 1, 0},
	{0xe0, 0xd0, 2, 0},
	{0xe0, 0x00, 2, 0},
	{0xe0, 0x00, 2, 0},
	{0xe0, 0x08, 2, 0},
	{0xe0, 0x07, 2, 0},
	{0xe0, 0x05, 2, 0},
	{0xe0, 0x29, 2, 0},
	{0xe0, 0x54, 2, 0},
	{0xe0, 0x41, 2, 0},
	{0xe0, 0x3c, 2, 0},
	{0xe0, 0x17, 2, 0},
	{0xe0, 0x15, 2, 0},
	{0xe0, 0x1a, 2, 0},
	{0xe0, 0x20, 2, 0},

	{0xe1, 0xe1, 1, 0},
	{0xe1, 0xd0, 2, 0},
	{0xe1, 0x00, 2, 0},
	{0xe1, 0x00, 2, 0},
	{0xe1, 0x08, 2, 0},
	{0xe1, 0x07, 2, 0},
	{0xe1, 0x04, 2, 0},
	{0xe1, 0x29, 2, 0},
	{0xe1, 0x44, 2, 0},
	{0xe1, 0x42, 2, 0},
	{0xe1, 0x3b, 2, 0},
	{0xe1, 0x16, 2, 0},
	{0xe1, 0x15, 2, 0},
	{0xe1, 0x1b, 2, 0},
	{0xe1, 0x1f, 2, 0},

	{0x35, 0x35, 1, 0}, // TE on
	{0x35, 0x00, 2, 0}, // TE mode: 0, mode1; 1, mode2

	{0x29, 0x29, 1, 0}, //Display ON

	/* set window size*/
//	{0xcd, 0xcd, 1, 0},
	{0x2a, 0x2a, 1, 0},
	{0x2a, 0, 2, 0},
	{0x2a, 0, 2, 0},
	{0x2a, (239>> 8) & 0xff, 2, 0},
	{0x2a, 239 & 0xff, 2, 0},
#ifdef	CONFIG_TRULY_240X240_ROTATE_180
	{0x2b, 0x2b, 1, 0},
	{0x2b, ((320-240)>>8)&0xff, 2, 0},
	{0x2b, ((320-240)>>0)&0xff, 2, 0},
	{0x2b, ((320-1)>>8) & 0xff, 2, 0},
	{0x2b, ((320-1)>>0) & 0xff, 2, 0},
#else
	{0x2b, 0x2b, 1, 0},
	{0x2b, 0, 2, 0},
	{0x2b, 0, 2, 0},
	{0x2b, (239>> 8) & 0xff, 2, 0},
	{0x2b, 239 & 0xff, 2, 0},
#endif

//	{0xcd, 0xcd, 1, 0},
	{0x2c, 0x2c, 1, 0},
};

struct smart_lcd_data_table truly320320_update_table[] = {
	//{0x39, 0x39, 1, 0}, //idle mode on
	{0x00, 0x39, 1, 0}, ///
#if 1
	{0x00, 0xB0, 1, 0},
	{0x00, 0x00, 2, 0},
	{0x00, 0xB1, 1, 0},   //SETPower
	{0x00, 0x00, 2, 0},	  //Ref from 57-C BOE code
	{0x00, 0x14, 2, 0},//TRI XDK BT
	{0x00, 0x1E, 2, 0},//VRH
	{0x00, 0x1E, 2, 0},//NVRH
	{0x00, 0xC3, 2, 0},//GAS_EN  FP_DDVDH AP
	{0x00, 0xFF, 2, 1000},//FS1 FS0

	{0x00, 0xB4, 1, 0},   //SETCYC
	{0x00, 0x22, 2, 0},	  //2-dot
	{0x00, 0x40, 2, 0},//RTN
	{0x00, 0x03, 2, 0},//DIV //0x01,03 will smaller 0.2mA <//DIV   0x03 约5hz, 0x02 约10hz,0x01约20hz, 0x00约40hz>
	{0x00, 0x2A, 2, 0},//DUM
	{0x00, 0x2A, 2, 0},//DUM
	{0x00, 0x20, 2, 0},//GDON
	{0x00, 0x91, 2, 0},//GDOFF
	{0x00, 0xC0, 1, 0},   //
	{0x00, 0x24, 2, 0},	  //
	{0x00, 0x24, 2, 0},//OPON
	{0x00, 0x01, 2, 0},	// Gate Rising/Falling edge EQ
	{0x00, 0x0C, 2, 0},
	{0x00, 0xD4, 2, 0},
	{0x00, 0x08, 2, 0},

	{0x00, 0xC2, 1, 0},   //
	{0x00, 0x00, 2, 0},	  //
	{0x00, 0x08, 2, 0},
	{0x00, 0x07, 2, 0},	// Gate Rising/Falling edge EQ

	{0x00, 0xE3, 1, 0},   //
	{0x00, 0x2F, 2, 0},	  //
	{0x00, 0x03, 2, 0},
#endif
};

unsigned int truly320320_update_table_length = ARRAY_SIZE(truly320320_update_table);

struct smart_lcd_data_table truly320320_restore_table[] = {
	{0x00, 0x38, 1, 0}, ///Exit idle mode
#if 1
	{0x00, 0xB0, 1, 0},
	{0x00, 0x66, 2, 0},

	{0x00, 0xB1, 1, 0},   //SETPower
	{0x00, 0x00, 2, 0},	  //Ref from 57-C BOE code
	{0x00, 0x14, 2, 0},//TRI XDK BT
	{0x00, 0x1E, 2, 0},//VRH
	{0x00, 0x1E, 2, 0},//NVRH
	{0x00, 0xC3, 2, 0},//GAS_EN  FP_DDVDH AP
	{0x00, 0x44, 2, 1000},//FS1 FS0

	{0x00, 0xB4, 1, 0},   //SETCYC
	{0x00, 0x22, 2, 0},	  //2-dot
	{0x00, 0x40, 2, 0},//RTN
	{0x00, 0x00, 2, 0},//DIV
	{0x00, 0x2A, 2, 0},//DUM
	{0x00, 0x2A, 2, 0},//DUM
	{0x00, 0x20, 2, 0},//GDON
	{0x00, 0x91, 2, 0},//GDOFF	

	{0x00, 0xC0, 1, 0},   //
	{0x00, 0x60, 2, 0},	  //
	{0x00, 0x60, 2, 0},//OPON
	{0x00, 0x01, 2, 0},	//
	{0x00, 0x3C, 2, 0},
	{0x00, 0xC8, 2, 0},
	{0x00, 0x08, 2, 0},

	{0x00, 0xC2, 1, 0},   //
	{0x00, 0x00, 2, 0},	  //
	{0x00, 0x08, 2, 0},
	{0x00, 0x05, 2, 0},	//

	{0x00, 0xE3, 1, 0},   //
	{0x00, 0x17, 2, 0},	  //
	{0x00, 0x0F, 2, 0},
#endif
};

unsigned int truly320320_restore_table_length = ARRAY_SIZE(truly320320_restore_table);

static struct fb_videomode truly320320_videomode = {
	.name = "320x320",
	.refresh = 50,
	.xres = 320,
	.yres = 320,
	.pixclock = KHZ2PICOS(30000),
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

unsigned long truly320320_gram_cmd[] = {
	0x2c2c2c2c
};

struct jzfb_platform_data truly320320_jzfb_pdata = {
	.lcd.name = "truly320320",
	.lcd.resource_init = truly320320_res_init,
	.lcd.power_on = truly320320_power_on,
	.lcd.update_mode = truly320320_update_mode,

	.num_modes = 1,
	.modes = &truly320320_videomode,

	.lcd_type = LCD_TYPE_LCM,
	.bpp = 18,
	.width = 31,
	.height = 31,
	.pinmd = 0,

	.pixclk_falling_edge = 0,
	.date_enable_active_low = 0,

	.alloc_vidmem = 1,
	.lvds = 0,

	.smart_config.smart_type = SMART_LCD_TYPE_PARALLEL,
	.smart_config.cmd_width = SMART_LCD_CWIDTH_8_BIT_ONCE,
	.smart_config.data_width = SMART_LCD_DWIDTH_8_BIT_ONCE_PARALLEL_SERIAL,
	.smart_config.data_width2 = SMART_LCD_DWIDTH_8_BIT_THIRD_TIME_PARALLEL,
	.smart_config.clkply_active_rising = 0,
	.smart_config.rsply_cmd_high = 0,
	.smart_config.csply_active_high = 0,
	.smart_config.chip_select_valid_high = 0,
	/* write graphic ram command, in word, for example 8-bit bus, write_gram_cmd=C3C2C1C0. */
	.smart_config.write_gram_cmd = truly320320_gram_cmd,
	.smart_config.bus_width = 8,
	.smart_config.length_data_table =  ARRAY_SIZE(truly320320_data_table),
	.smart_config.data_table = truly320320_data_table,
	.smart_config.init = truly320320_init,
	.smart_config.disable_slcd_dma_restart_work_around = 1,
#if 0
	.smart_config.te.te_gpio = GPIO_LCD_TE,
	.smart_config.te.te_gpio_level = IRQF_TRIGGER_HIGH,
#endif
#if 0
	.dither_enable = 1,
	.dither.dither_red = 1, /* 6bit */
	.dither.dither_red = 1, /* 6bit */
	.dither.dither_red = 1, /* 6bit */
#else
	.dither_enable = 0,
#endif
};
