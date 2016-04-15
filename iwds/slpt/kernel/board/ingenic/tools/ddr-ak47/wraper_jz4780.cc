/*
 * Copyright (C) 2013 Ingenic Semiconductor Co., Ltd.
 * Authors: Duke Fong <duke@dukelec.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 3 of the License, or (at your option) any later version.
 */

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include "calc_intf.h"

extern "C" void *ddr_params_creator(int cpu_type, int ddr_type,
		void *cpu_struct, void *ddr_struct);

using namespace std;

static void calc_xburst_ingenic_jz4780_ddr3(bool exec)
{
	struct jz4780_dram_params cpu_params;
	struct ddr3_params ddr3_params;
	struct dwc_ddr_config *out;
	string str;

	// read cpu ini to struct (print or not)
	cpu_params.freq = ini_get_value(ini_cpu, "freq", unit_Hz);
	str += stringf("freq: %dHz\n", cpu_params.freq);
	cpu_params.ddr_odt = ini_get_value(ini_cpu, "ddr_odt", unit_null);
	str += stringf("ddr_odt: %d\n", cpu_params.ddr_odt);
	cpu_params.phy_odt = ini_get_value(ini_cpu, "phy_odt", unit_null);
	str += stringf("phy_odt: %d\n", cpu_params.phy_odt);
	cpu_params.dq_odt = ini_get_value(ini_cpu, "dq_odt", unit_null);
	str += stringf("dq_odt: %d\n", cpu_params.dq_odt);
	cpu_params.dqs_odt = ini_get_value(ini_cpu, "dqs_odt", unit_null);
	str += stringf("dqs_odt: %d\n", cpu_params.dqs_odt);
	cpu_params.pu_ohm = ini_get_value(ini_cpu, "pu_ohm", unit_null);
	str += stringf("pu_ohm: 0x%x\n", cpu_params.pu_ohm);
	cpu_params.pd_ohm = ini_get_value(ini_cpu, "pd_ohm", unit_null);
	str += stringf("pd_ohm: 0x%x\n", cpu_params.pd_ohm);
	cpu_params.cs0 = ini_get_value(ini_cpu, "cs0", unit_null);
	str += stringf("cs0: %d\n", cpu_params.cs0);
	cpu_params.cs1 = ini_get_value(ini_cpu, "cs1", unit_null);
	str += stringf("cs1: %d\n", cpu_params.cs1);
	cpu_params.dw32 = ini_get_value(ini_cpu, "dw32", unit_null);
	str += stringf("dw32: %d\n", cpu_params.dw32);
	cpu_params.dll = ini_get_value(ini_cpu, "dll", unit_null);
	str += stringf("dll: %d\n", cpu_params.dll);
	cpu_params.div = ini_get_value(ini_cpu, "div", unit_null);
	str += stringf("div: %d\n", cpu_params.div);
	cpu_params.col = ini_get_value(ini_cpu, "col", unit_null);
	str += stringf("col: %d\n", cpu_params.col);
	cpu_params.row = ini_get_value(ini_cpu, "row", unit_null);
	str += stringf("row: %d\n", cpu_params.row);
	cpu_params.bank8 = ini_get_value(ini_cpu, "bank8", unit_null);
	str += stringf("bank8: %d\n", cpu_params.bank8);

	if (!exec) {
		cout << str;
		return;
	} else
		str.clear();

	// read ddr ini to struct
	ini_parse_ddr3(ddr3_params);

	// invoke calculate function
	out = (struct dwc_ddr_config *) ddr_params_creator(CPU_TYPE_JZ4780, DDR_TYPE_DDR3,
			&cpu_params, &ddr3_params);

	str += stringf("ddrc_cfg: 0x%08x\n", out->ddrc_cfg);
	str += stringf("ddrc_ctrl: 0x%08x\n", out->ddrc_ctrl);
	str += stringf("ddrc_refcnt: 0x%08x\n", out->ddrc_refcnt);
	str += stringf("ddrc_mmap0: 0x%08x\n", out->ddrc_mmap0);
	str += stringf("ddrc_mmap1: 0x%08x\n", out->ddrc_mmap1);
	str += stringf("ddrc_timing1: 0x%08x\n", out->ddrc_timing1);
	str += stringf("ddrc_timing2: 0x%08x\n", out->ddrc_timing2);
	str += stringf("ddrc_timing3: 0x%08x\n", out->ddrc_timing3);
	str += stringf("ddrc_timing4: 0x%08x\n", out->ddrc_timing4);
	str += stringf("ddrc_timing5: 0x%08x\n", out->ddrc_timing5);
	str += stringf("ddrc_timing6: 0x%08x\n", out->ddrc_timing6);
	str += stringf("ddrp_dcr: 0x%08x\n", out->ddrp_dcr);
	str += stringf("ddrp_mr0: 0x%08x\n", out->ddrp_mr0);
	str += stringf("ddrp_mr1: 0x%08x\n", out->ddrp_mr1);
	str += stringf("ddrp_mr2: 0x%08x\n", out->ddrp_mr2);
	str += stringf("ddrp_mr3: 0x%08x\n", out->ddrp_mr3);
	str += stringf("ddrp_ptr0: 0x%08x\n", out->ddrp_ptr0);
	str += stringf("ddrp_ptr1: 0x%08x\n", out->ddrp_ptr1);
	str += stringf("ddrp_ptr2: 0x%08x\n", out->ddrp_ptr2);
	str += stringf("ddrp_pgcr: 0x%08x\n", out->ddrp_pgcr);
	str += stringf("ddrp_odtcr: 0x%08x\n", out->ddrp_odtcr);
	str += stringf("ddr_odt: 0x%08x\n", out->ddr_odt);
	str += stringf("ddr_size0: 0x%08x\n", out->ddr_size0);
	str += stringf("ddr_size1: 0x%08x\n", out->ddr_size1);
	cout << str;

	if (!elf.empty()) {
		cout << "\nWriting to ELF...\n";
		elf_write("ddrc_cfg", out->ddrc_cfg);
		elf_write("ddrc_ctrl", out->ddrc_ctrl);
		elf_write("ddrc_refcnt", out->ddrc_refcnt);
		elf_write("ddrc_mmap0", out->ddrc_mmap0);
		elf_write("ddrc_mmap1", out->ddrc_mmap1);
		elf_write("ddrc_timing1", out->ddrc_timing1);
		elf_write("ddrc_timing2", out->ddrc_timing2);
		elf_write("ddrc_timing3", out->ddrc_timing3);
		elf_write("ddrc_timing4", out->ddrc_timing4);
		elf_write("ddrc_timing5", out->ddrc_timing5);
		elf_write("ddrc_timing6", out->ddrc_timing6);
		elf_write("ddrp_dcr", out->ddrp_dcr);
		elf_write("ddrp_mr0", out->ddrp_mr0);
		elf_write("ddrp_mr1", out->ddrp_mr1);
		elf_write("ddrp_mr2", out->ddrp_mr2);
		elf_write("ddrp_mr3", out->ddrp_mr3);
		elf_write("ddrp_ptr0", out->ddrp_ptr0);
		elf_write("ddrp_ptr1", out->ddrp_ptr1);
		elf_write("ddrp_ptr2", out->ddrp_ptr2);
		elf_write("ddrp_pgcr", out->ddrp_pgcr);
		elf_write("ddrp_odtcr", out->ddrp_odtcr);
		elf_write("ddr_odt", out->ddr_odt);
		elf_write("ddr_size0", out->ddr_size0);
		elf_write("ddr_size1", out->ddr_size1);
	}
}

static calc_intf intf("xburst", "ingenic", "jz4780", "ddr3", calc_xburst_ingenic_jz4780_ddr3);
