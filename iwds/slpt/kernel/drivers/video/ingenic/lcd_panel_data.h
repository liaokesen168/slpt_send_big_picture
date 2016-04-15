/**
 * jzfb platform data declarations.
 *
 * If you have a new lcd device, modify lcd_panel_data.h, lcd_panel_data.c
 *
 * lcd_panel_data.h : save jzfb platform data declaration.
 * lcd_panel_data.c : contain jzfb platform data pointer.
 *
 */

#ifndef _LCD_PANEL_DATA_H_
#define _LCD_PANEL_DATA_H_

#include <jzfb.h>

#ifdef CONFIG_SLCD_SEPS645B
extern struct jzfb_platform_data seps645b_jzfb_pdata;
#endif

#ifdef CONFIG_SLCD_TRULY240240
extern struct jzfb_platform_data truly240240_jzfb_pdata;
#endif

#ifdef CONFIG_LCD_X163
struct jzfb_platform_data auo_x163_jzfb_pdata;
#endif

#ifdef CONFIG_SLCD_TRULY320320
extern struct jzfb_platform_data truly320320_jzfb_pdata;
#endif

#ifdef CONFIG_LCD_BOE_TFT320320
extern struct jzfb_platform_data boe_tft320320_jzfb_pdata;
#endif

#ifdef CONFIG_LCD_AUO_H139BLN01
extern struct jzfb_platform_data auo_h139bln01_jzfb_pdata;
#endif

#ifdef CONFIG_LCD_EDO_E1392AM1
extern struct jzfb_platform_data edo_e1392am1_jzfb_pdata;
#endif

#ifdef CONFIG_LCD_H160_TFT320320
extern struct jzfb_platform_data h160_tft320320_jzfb_pdata;
#endif

#ifdef CONFIG_LCD_ARS_NT35350
extern struct jzfb_platform_data ars_nt35350_jzfb_pdata;
#endif

#endif /* _LCD_PANEL_DATA_H_ */
