/*
 * Copyright (C) 2013 Ingenic Semiconductor Co., Ltd.
 * Authors: Kage Shen <kkshen@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 3 of the License, or (at your option) any later version.
 */

#include <common.h>
#include <asm/io.h>
#include <linux/lcd_board_info.h>
#include <asm/arch/jz4775_gpio.h>

DECLARE_GLOBAL_DATA_PTR;

static void gpio_init(void)
{
}

int jz4780_mmc_init(int dev_index, uint host_caps_mask, uint f_max);

int board_mmc_init(bd_t *bis)
{
	jz4780_mmc_init(0, 0, 0);
	return 0;
}

int board_early_init_f(void)
{
	/* Interrupt Controller Mask Clear Register */
	*(volatile u32 *)(0xb000100c) = 0x00040000;
	*(volatile u32 *)(0xb000102c) = 0x0f0f0000;

	gpio_init();
#if 0
	calc_clocks();  /* calc the clocks */
	rtc_init();     /* init rtc on any reset */
#endif
	*(volatile u32 *)(0xb0003000) |= 1;
	*(volatile u32 *)(0xb0003048) &= 0xfffffffe;

	return 0;
}

/* U-Boot common routines */
int checkboard(void)
{
	printf("Board: s2122b (Ingenic XBurst Jz4775 SoC)\n");

	return 0;
}

struct lcd_board_info lcd_board_info = {
	.lcd = {
		.name = "truly240240",
		.need_init = 0,
	},
	.io_mcu = {
		.cs = GPIO_PC(9),
	},
};
