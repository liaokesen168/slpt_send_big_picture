#include <config.h>
#include <common.h>

#include <slpt_app.h>
#include <slpt.h>
#include <fb_struct.h>
#include <low_battery.h>

extern unsigned int alpha32;
extern struct fb_region current_fb_region;

/*
 * region info
 */
static struct slpt_app_res region_info_res[] = {
	SLPT_RES_INT_DEF("base", current_fb_region.base),
	SLPT_RES_INT_DEF("bpp", current_fb_region.bpp),
	SLPT_RES_INT_DEF("xres", current_fb_region.xres),
	SLPT_RES_INT_DEF("yres", current_fb_region.yres),
	SLPT_RES_INT_DEF("pixels_per_line", current_fb_region.pixels_per_line),
};

/*
 * low battery
 *
 * low battery move to setting/key
 */


/*
 * setting
 */
static struct slpt_app_res slpt_settting_res[] = {
	SLPT_RES_INT_DEF("alpha32", alpha32),
	SLPT_RES_DIR_DEF("region", region_info_res),
	SLPT_RES_EMPTY_DEF("key", SLPT_RES_DIR),
};

extern int pr_debug_battery;
extern int pr_debug_key;
extern int pr_debug_view_display;
extern int pr_debug_picture;

/*
 * debug
 */
static struct slpt_app_res slpt_debug_res[] = {
	/* SLPT_RES_INT_DEF("battery", pr_debug_battery), */
	SLPT_RES_INT_DEF("key", pr_debug_key),
	SLPT_RES_INT_DEF("view-display", pr_debug_view_display),
	SLPT_RES_INT_DEF("picture", pr_debug_picture),
};

/**
 * slpt res top dir
 */
static struct slpt_app_res slpt_top_res[] = {
	SLPT_RES_EMPTY_DEF("devices", SLPT_RES_DIR),         /* device directory */
	SLPT_RES_DIR_DEF("setting", slpt_settting_res),        /* setting directory */
	SLPT_RES_DIR_DEF("debug", slpt_debug_res), 			/* debug directory */
	SLPT_RES_EMPTY_DEF("pictures", SLPT_RES_DIR),
	SLPT_RES_EMPTY_DEF("clock", SLPT_RES_DIR),
	SLPT_RES_EMPTY_DEF("test", SLPT_RES_DIR),
};
SLPT_REGISTER_RES_ARR(slpt_top_res, slpt_top_res);
