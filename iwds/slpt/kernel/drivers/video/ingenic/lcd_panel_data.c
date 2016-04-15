/**
 * jzfb platform data pointers.
 *
 * If you have a new lcd device, modify lcd_panel_data.h, lcd_panel_data.c
 *
 * lcd_panel_data.h : save jzfb platform data declaration.
 * lcd_panel_data.c : contain jzfb platform data pointer.
 *
 */

#include <config.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <common.h>
#include <slpt.h>
#include <jzfb.h>
#include <malloc.h>
#include <linux/pr_info.h>

#include "lcd_panel_data.h"

struct jzfb_platform_data *jzfb_pdata[] = {
#ifdef CONFIG_SLCD_SEPS645B
	&seps645b_jzfb_pdata,
#endif
#ifdef CONFIG_SLCD_TRULY240240
	&truly240240_jzfb_pdata,
#endif
#ifdef CONFIG_LCD_X163
	&auo_x163_jzfb_pdata,
#endif
#ifdef CONFIG_SLCD_TRULY320320
	&truly320320_jzfb_pdata,
#endif
#ifdef CONFIG_LCD_BOE_TFT320320
	&boe_tft320320_jzfb_pdata,
#endif
#ifdef CONFIG_LCD_AUO_H139BLN01
	&auo_h139bln01_jzfb_pdata,
#endif
#ifdef CONFIG_LCD_EDO_E1392AM1
	&edo_e1392am1_jzfb_pdata,
#endif
#ifdef CONFIG_LCD_H160_TFT320320
	&h160_tft320320_jzfb_pdata,
#endif
#ifdef CONFIG_LCD_ARS_NT35350
	&ars_nt35350_jzfb_pdata,
#endif
};

struct jzfb_platform_data *get_jzfb_platform_data(const char *name) {
	struct jzfb_platform_data **pdata = jzfb_pdata;
	unsigned int n = ARRAY_SIZE(jzfb_pdata);
	int i;

	for (i = 0; i < n; ++i) {
		if (!pdata[i])
			continue;
		if (!name || !pdata[i]->lcd.name) {
			if (name == pdata[i]->lcd.name)
				return pdata[i];
		} else {
			if (!strcmp(name, pdata[i]->lcd.name)) {
				return pdata[i];
			}
		}
	}

	pr_err("jzfb: NO supported lcd platform data:%s\n",name);
	return NULL;
}
