/**
 * jzfb platform data : auox163 mipi oled
 *
 */
#include <config.h>
#include <common.h>
#include <jzfb.h>
#include <jz_dsim.h>

extern int auo_x163_init(void);
extern int auo_x163_res_init(void);
extern int auo_x163_power_on(int on);
extern int auo_x163_update_mode(int update);

struct fb_videomode auo_x163_jzfb_videomode = {
	.name = "auo_x163-lcd",
	.refresh = 60,
	.xres = 320,
	.yres = 320,
	.pixclock = KHZ2PICOS(5760),
	.left_margin = 0,
	.right_margin = 0,
	.upper_margin = 0,
	.lower_margin = 0,
	.hsync_len = 0,
	.vsync_len = 0,
	.sync = ~FB_SYNC_HOR_HIGH_ACT & ~FB_SYNC_VERT_HIGH_ACT,
	.vmode = FB_VMODE_NONINTERLACED,
	.flag = 0,
};

unsigned long auo_x163_cmd_buf[]= {
	0x2C2C2C2C,
};

struct jzdsi_data auo_x163_jzdsi_pdata = {
	.modes = &auo_x163_jzfb_videomode,
	.video_config.no_of_lanes = 1,
	.video_config.virtual_channel = 0,
	.video_config.color_coding = COLOR_CODE_24BIT,
	.video_config.byte_clock = DEFAULT_DATALANE_BPS / 8,
	.video_config.video_mode = VIDEO_BURST_WITH_SYNC_PULSES,
	.video_config.receive_ack_packets = 0,	/* enable receiving of ack packets */
	.video_config.is_18_loosely = 0, /*loosely: R0R1R2R3R4R5__G0G1G2G3G4G5G6__B0B1B2B3B4B5B6, not loosely: R0R1R2R3R4R5G0G1G2G3G4G5B0B1B2B3B4B5*/
	.video_config.data_en_polarity = 1,

	.dsi_config.max_lanes = 1,
	.dsi_config.max_hs_to_lp_cycles = 100,
	.dsi_config.max_lp_to_hs_cycles = 40,
	.dsi_config.max_bta_cycles = 4095,
	.dsi_config.color_mode_polarity = 1,
	.dsi_config.shut_down_polarity = 1,
};

struct jzfb_platform_data auo_x163_jzfb_pdata = {
	.lcd.name = "auo_x163-lcd",
	.lcd.resource_init = auo_x163_res_init,
	.lcd.power_on = auo_x163_power_on,
	.lcd.update_mode = auo_x163_update_mode,

	.num_modes = 1,
	.modes = &auo_x163_jzfb_videomode,
	.dsi_pdata = &auo_x163_jzdsi_pdata,
	.mipi_dsi = 1,

	.lcd_type = LCD_TYPE_SLCD,
	.bpp = 24,
	.width = 31,
	.height = 31,

	.smart_config.clkply_active_rising = 0,
	.smart_config.rsply_cmd_high = 0,
	.smart_config.csply_active_high = 0,
	.smart_config.write_gram_cmd = auo_x163_cmd_buf,
	.smart_config.length_cmd = ARRAY_SIZE(auo_x163_cmd_buf),
	.smart_config.bus_width = 8,
	.dither_enable = 1,
	.dither.dither_red = 1,	/* 6bit */
	.dither.dither_red = 1,	/* 6bit */
	.dither.dither_red = 1,	/* 6bit */
};
