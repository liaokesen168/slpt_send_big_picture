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
#endif
#include <asm/arch/board_special.h>

#include "../common/init_sequence.h"

struct boot_power_parameter power_param = {
	.act8600_enabled = 1,
	.out_vol = {
		1200,
	},
};

PFSV1_DEF_PARAM_BEGIN(Power, "Power subsystem",
		"text: Board power configuration")
	PFSV1_DEF_PARAM(power_param.act8600_enabled, "Enable PMU act8600",
			int, "rw",
			"text: flag enable pmu act8600"
			"value:[1(\"enable\"), 0(\"disable\")]")
	PFSV1_DEF_PARAM_BLOCK_BEGIN(power_param, "Configuration",
			struct boot_power_parameter,
			"text: Board power configuration")
		PFSV1_DEF_PARAM(power_param.out_vol[0], "Channel 1 Output Voltage",
				int, "rw",
				"text: channel 1 output voltage (mV)\n"
				"value: [0], [600:25:1175], [1200:50:2350], [2400:100:3900]")
		PFSV1_DEF_PARAM(power_param.out_vol[1], "Channel 2 Output Voltage",
				int, "rw",
				"text: channel 2 output voltage (mV)\n"
				"value: [0], [600:25:1175], [1200:50:2350], [2400:100:3900]")
		PFSV1_DEF_PARAM(power_param.out_vol[2], "Channel 3 Output Voltage",
				int, "rw",
				"text: channel 3 output voltage (mV)\n"
				"value: [0], [600:25:1175], [1200:50:2350], [2400:100:3900]")
		PFSV1_DEF_PARAM(power_param.out_vol[3], "Channel 4 Output Voltage",
				int, "rw",
				"text: channel 4 output voltage (mV)\n"
				"value: [0], [3000:100:12500], [12600:200:18800], [19000:400:41400]")
		PFSV1_DEF_PARAM(power_param.out_vol[4], "Channel 5 Output Voltage",
				int, "rw",
				"text: channel 5 output voltage (mV)\n"
				"value: [0], [600:25:1175], [1200:50:2350], [2400:100:3900]")
		PFSV1_DEF_PARAM(power_param.out_vol[5], "Channel 6 Output Voltage",
				int, "rw",
				"text: channel 6 output voltage (mV)\n"
				"value: [0], [600:25:1175], [1200:50:2350], [2400:100:3900]")
		PFSV1_DEF_PARAM(power_param.out_vol[6], "Channel 7 Output Voltage",
				int, "rw",
				"text: channel 7 output voltage (mV)\n"
				"value: [0], [600:25:1175], [1200:50:2350], [2400:100:3900]")
		PFSV1_DEF_PARAM(power_param.out_vol[7], "Channel 8 Output Voltage",
				int, "rw",
				"text: channel 8 output voltage (mV)\n"
				"value: [0], [600:25:1175], [1200:50:2350], [2400:100:3900]")
	PFSV1_DEF_PARAM_BLOCK_END(power_param)
PFSV1_DEF_PARAM_END(Power)

PFSV1_DEF_HELP_ZH_BEGIN(Power)
    PFSV1_DEF_HELP(power_param,
			"PMU器件选择和各路输出电压的设置, 但需要注意每一路输出的"
			"电压范围, 单位为毫伏(mV)")
PFSV1_DEF_HELP_END(Power)

