#ifndef __BOARD_SPECIAL_H__
#define __BOARD_SPECIAL_H__

#include <asm/arch/jz4780_nand.h>

struct boot_misc_parameter {
	unsigned int extal_clock;   /* extal clock */
	unsigned int cpu_freq;      /* cpu frequency */
	unsigned int mem_size;      /* Memory Size */
	unsigned int malloc_len;
	unsigned int uart_num;
	unsigned char bootargs[512];
	unsigned char bootcmd[512];
};

struct boot_power_parameter {
	unsigned int act8600_enabled;
	unsigned int out_vol[8];
};

extern struct boot_misc_parameter misc_param;
extern struct boot_parameter board_boot_para;
extern struct boot_power_parameter power_param;
extern struct jz4780_nand_platform_data nand_platform_data;
#endif

