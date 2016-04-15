/*
 * Copyright (C) 2013 Ingenic Semiconductor Co., Ltd.
 * Authors: Kage Shen <kkshen@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the License, or (at your
 * option) any later version.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <config.h>
#include <common.h>
#include <pfsv1.h>

#include <linux/err.h>
#include <linux/list.h>
#include <regulator.h>

#if defined(CONFIG_JZ4780)
#include <asm/arch/jz4780_pfsv1.h>
#elif defined(CONFIG_JZ4775)
#include <asm/arch/jz4775_pfsv1.h>
#elif defined(CONFIG_M200)
#include <asm/arch/m200_pfsv1.h>
#endif
#include <asm/arch/board_special.h>

#include "../common/init_sequence.h"

struct boot_misc_parameter misc_param = {
	.extal_clock = 24,
	.cpu_freq = 816,
	.mem_size = CONFIG_MEM_SIZE,
	.malloc_len = CONFIG_SYS_MALLOC_LE,
	.uart_num = 3,
	.bootargs = SPECIAL_BOOTARGS,
	.bootcmd = SPECIAL_BOOTCOMMAND,
};

PFSV1_DEF_PARAM_BEGIN(Misc, "Boot misc configuration",
		"text: System misc configuration")
	PFSV1_DEF_PARAM(misc_param.extal_clock, "External crystal",
			int, "rw",
			"text: chip external crystal (MHz)")
	PFSV1_DEF_PARAM(misc_param.cpu_freq, "Cpu frequency",
			int, "rw",
			"text: cpu frequency (MHz)")
	PFSV1_DEF_PARAM(misc_param.mem_size, "Memory size",
			int, "rw",
			"text: DRAM size on board (byte)")
	PFSV1_DEF_PARAM(misc_param.uart_num, "Print uart",
			int, "rw",
			"text: current uart number")
	PFSV1_DEF_PARAM(misc_param.bootargs, "Boot argument",
			char [], "rw",
			"text: kernel boot argument")
	PFSV1_DEF_PARAM(misc_param.bootcmd, "Boot command",
			char [], "rw",
			"text: uboot command")
PFSV1_DEF_PARAM_END(Misc)

PFSV1_DEF_HELP_ZH_BEGIN(Misc)
	PFSV1_DEF_HELP(misc_param.extal_clock,
			"硬件电路上, 外部时钟(晶体)频率(MHz)")
	PFSV1_DEF_HELP(misc_param.cpu_freq,
			"CPU启动时频率值(MHz)")
	PFSV1_DEF_HELP(misc_param.mem_size,
            "硬件电路上, 内存容量")
	PFSV1_DEF_HELP(misc_param.uart_num,
            "启动时输出串口号")
	PFSV1_DEF_HELP(misc_param.bootargs,
            "kernel启动时uboot所传递的参数")
	PFSV1_DEF_HELP(misc_param.bootcmd,
            "uboot启动命令")
PFSV1_DEF_HELP_END(Misc)

