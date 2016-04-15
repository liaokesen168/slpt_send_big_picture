/*
 * Copyright (C) 2013 Ingenic Semiconductor Co., Ltd.
 * Authors: Kage Shen <kkshen@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 3 of the License, or (at your option) any later version.
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>

#include <asm/arch/board_special.h>

void enable_interrupts(void)
{

}

int disable_interrupts(void)
{
	return 0;
}

void pll_init(void)
{
	/* init at spl so nothing TODO now */
}

void sdram_init(void)
{
	/* init at spl so nothing TODO now */
}

DECLARE_GLOBAL_DATA_PTR;

void calc_clocks(void)
{

}

void rtc_init(void)
{

}

/* U-Boot common routines */
phys_size_t initdram(int board_type)
{
	/* return memory size */
	return misc_param.mem_size;
}

