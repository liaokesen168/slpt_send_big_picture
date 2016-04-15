/*
 *  Copyright (C) 2013 Fighter Sun <wanmyqawdr@126.com>
 *  board initialize sequence
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General  Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <serial.h>
#include <stdio_dev.h>
#include <version.h>
#include <net.h>
#include <environment.h>
#include <nand.h>
#include <onenand_uboot.h>
#include <spi.h>
#include <mmc.h>
#include <regulator.h>

#ifdef CONFIG_BITBANGMII
#include <miiphy.h>
#endif

#ifdef CONFIG_JZ_FB
#include <fb_struct.h>
#endif

#include <asm/arch/board_special.h>

#include "init_sequence.h"

extern int jz4780_clk_init(void);
extern int gpemc_init(void);
extern int bch_init(void);
extern int jz4780_dma_init(void);
extern int jz4780_board_nand_init(void);
extern int act8600_regulator_init(void);
extern int jz4780_sadc_init(void);
extern int jz4780_battery_init(void);
extern int jz4780_udc_init(void);
extern int udc_rio_startup(void);
extern void slpt_key_init_all(void);

DECLARE_GLOBAL_DATA_PTR;

static char *failed = "*** failed ***\n";

int __board_early_init_f(void)
{
	/*
	 * Nothing to do in this dummy implementation
	 */
	return 0;
}

int board_early_init_f(void)
	__attribute__((weak, alias("__board_early_init_f")));

static int init_func_ram(void)
{
#ifdef	CONFIG_BOARD_TYPES
	int board_type = gd->board_type;
#else
	int board_type = 0;	/* use dummy arg */
#endif
	puts("DRAM:  ");

	gd->ram_size = initdram(board_type);
	if (gd->ram_size > 0) {
		print_size(gd->ram_size, "\n");
		return 0;
	}
	puts(failed);
	return 1;
}

static int display_banner(void)
{
	printf("\n%s\n", version_string);
	return 0;
}

#ifndef CONFIG_SYS_NO_FLASH
static void display_flash_config(ulong size)
{
	puts("Flash: ");
	print_size(size, "\n");
}
#endif

static int jz4780_flash_init(void)
{
#ifndef CONFIG_SLPT
	bd_t *bd = gd->bd;

#ifndef CONFIG_SYS_NO_FLASH
	unsigned long size;
	/* configure available FLASH banks */
	size = flash_init();
	display_flash_config(size);
	bd->bi_flashstart = CONFIG_SYS_FLASH_BASE;
	bd->bi_flashsize = size;

#if CONFIG_SYS_MONITOR_BASE == CONFIG_SYS_FLASH_BASE
	bd->bi_flashoffset = monitor_flash_len;	/* reserved area for U-Boot */
#else
	bd->bi_flashoffset = 0;
#endif
#else
	bd->bi_flashstart = 0;
	bd->bi_flashsize = 0;
	bd->bi_flashoffset = 0;
#endif
#endif

	return 0;
}

static int jz4780_mmc_initialize(void)
{
#ifdef CONFIG_GENERIC_MMC
	puts("MMC:   ");
	mmc_initialize(gd->bd);
#endif

	return 0;
}

static int jz4780_spi_init(void)
{
#ifdef CONFIG_CMD_SPI
	puts("SPI:   ");
	spi_init();		/* go init the SPI */
	puts("ready\n");
#endif

	return 0;
}

static int jz4780_bb_miiphy_init(void)
{
#ifdef CONFIG_BITBANGMII
	bb_miiphy_init();
#endif

	return 0;
}

static int jz4780_eth_initialize(void)
{
#if defined(CONFIG_CMD_NET)
	puts("Net:   ");
	eth_initialize(gd->bd);
#endif

	return 0;
}

static int jzfb_init(void)
{
#ifdef CONFIG_JZ_FB
	lcd_bm_init_data();
#endif

	return 0;
}

static int slpt_init(void) {
#ifdef CONFIG_SLPT
	slpt_key_init_all();
#endif
	return 0;
}

static int jz4780_env_relocate(void)
{
	env_relocate();
	return 0;
}

static int jz4780_env_init(void)
{
	setenv("bootcmd", (char *)&misc_param.bootcmd);
	setenv("bootargs", (char *)&misc_param.bootargs);

	return 0;
}

static int jz4780_power_init(void)
{
#ifndef CONFIG_SLPT
	u32 i;
	char tmp[12];
	struct regulator *reg;

	if(power_param.act8600_enabled) {
		printf("board PMU act8600 init output\n");
	} else
		return 0;

	for(i = 0; i < (sizeof(power_param.out_vol) / 4); i++) {
		sprintf(tmp, "OUT%d", i + 1);
		reg = regulator_get(tmp);

		if (power_param.out_vol[i] == 0) {
			continue;
		}

		if(reg != NULL)
			regulator_set_voltage(reg,
					power_param.out_vol[i] * 1000,
					power_param.out_vol[i] * 1000);
	}
#endif

	return 0;
}

static int jz4780_jumptable_init(void)
{
	jumptable_init();
	return 0;
}

#if defined(CONFIG_NAND_JZ4780) && defined(CONFIG_MTD_PARTITIONS)
static int jz4780_init_nand_default_partitions(void)
{
	run_command("mtdparts default", 0);
	printk("initialize default NAND partitions done.\n");

	return 0;
}
#endif

int run_initcall_level(init_fnc_t init_sequence[], gd_t *id)
{
	init_fnc_t *init_fnc_ptr;
	int ret;

	gd = id;

	for (init_fnc_ptr = init_sequence;
			*init_fnc_ptr; ++init_fnc_ptr) {
		debug("initcall: %p\n", *init_fnc_ptr);
		ret = (*init_fnc_ptr)();
		if (ret) {
			printk("initcall sequence %p failed"
					" at call %p, return: %d\n",
			      init_sequence, *init_fnc_ptr, ret);
		}
	}
	return 0;
}

init_fnc_t init_sequence_f[] = {
	board_early_init_f,
	timer_init,
	env_init,           /* initialize environment */
#ifdef CONFIG_INCA_IP
	incaip_set_cpuclk,  /* set cpu clock according to env. variable */
#endif
	serial_init,        /* serial communications setup */

	console_init_f,
	display_banner,     /* say that we are here */
	checkboard,
	init_func_ram,
	NULL,
};

init_fnc_t init_sequence_r_stage0[] = {
#ifdef CONFIG_ACT8600
	act8600_regulator_init,
#endif
	jz4780_power_init,
//	jz4780_clk_init,
//	jz4780_dma_init,
//	jz4780_sadc_init,
//	jz4780_battery_init,
//	gpemc_init,
//	bch_init,
	jz4780_flash_init,
	NULL,
};

init_fnc_t init_sequence_r_stage1[] = {
#ifdef CONFIG_NAND_JZ4780
	jz4780_board_nand_init,
#endif
	jz4780_mmc_initialize,
	jz4780_spi_init,
	jz4780_bb_miiphy_init,
	jz4780_eth_initialize,
#if (defined(CONFIG_JZ4780_USB) || defined(CONFIG_JZ4775_USB) || defined(CONFIG_M200_USB))
	jz4780_udc_init,
#endif
	NULL,
};

init_fnc_t init_sequence_r_stage2[] = {
	slpt_init,
//	jz4780_power_init,
	jz4780_clk_init,
//	jz4780_sadc_init,
	jz4780_env_relocate,
	stdio_init,
	jz4780_jumptable_init,
	console_init_r,
	jz4780_env_init,
#if (defined(CONFIG_JZ4775_USB) || defined(CONFIG_M200_USB))
	udc_rio_startup,
#endif
#ifdef CONFIG_JZ_FB
	jzfb_init,
#endif
	NULL,
};

init_fnc_t init_sequence_r_stage3[] = {
#if defined(CONFIG_NAND_JZ4780) && defined(CONFIG_MTD_PARTITIONS)
	jz4780_init_nand_default_partitions,
#endif
	NULL,
};
