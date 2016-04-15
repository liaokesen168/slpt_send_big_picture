/*
 * Ingenic ddr parameters creator.
 * suppert: jz4780/ddr3
 *          jz4775/ddr3/lpddr
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
 *         Kage Shen <kkshen@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "ddr_param.h"
#include "ddr_register.h"
#include "../../spl/ddr_dwc.h"

#undef ddr_err
//#define ddr_err(fmt, args...) do{}while(0)
#define ddr_err(fmt, args...) fprintf(stderr, fmt, ##args)
#define debug_msg(fmt, args...) fprintf(stdout, fmt, ##args)

#ifndef max
    #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
    #define min(a,b) (((a) > (b)) ? (b) : (a))
#endif

#define BETWEEN(T, a, b) do { if (T < min(a, b)) T = min(a, b); \
                            if (T > max(a, b)) T = max(a, b); } while(0)

struct tck {
	uint32_t ps;
	uint32_t ns;
};

struct size {
	uint32_t chip0;
	uint32_t chip1;
};

struct ddr_params {
	uint32_t cpu_type; /* cpu type */
	uint32_t ddr_type; /* dram type */
	uint32_t freq; /* dram frequency */
	/* ODT parameters configuration */
	uint32_t ddr_odt; /* dram Soc&ddr odt config */
	uint32_t phy_odt; /* Soc ddr phy odt config */
	uint32_t dq_odt; /* phy DQ pin config */
	uint32_t dqs_odt; /* phy DQS pin config */
	uint32_t pu_ohm; /* impedance PULLUP ohm */
	uint32_t pd_ohm; /* impedance PULLDOWN ohm */

	/* Chip Select */
	uint32_t cs0; /* CSENx: whether a ddr chip exists
                     0: un-used, 1: used */
	uint32_t cs1;
	uint32_t dw32; /* 0: 16-bit data width, 1: 32-bit data width */
	uint32_t dll;  /* ddr phy dll clock enable flag */
	uint32_t div; /* DDR Auto-Refresh Counter clock Divider */

	/* DDR configuration */
	uint32_t bl; /* Burst length */
	uint32_t tCL; /* CAS latency */
	uint32_t col; /* column address */
	uint32_t row; /* row address */
	uint32_t bank8; /* Banks each chip: 0: 4banks, 1: 8banks */

	uint32_t tCWL; /* CAS Write Latency */
	uint32_t tAL; /* tCK, Additive Latency */
	uint32_t tDLLLOCK; /* DLL locking time. default 512 */
	uint32_t tXSDLL;
	uint32_t tMOD; /* Load mode update delay (DDR3 only) */
	uint32_t tXPDLL;

	uint32_t tREFI; /* Refresh period: 4096 refresh cycles/64ms */
	uint32_t tDLLSRST; /* In PHY PTR0 */

	/* DDR controller timing1 register */
	uint32_t tWR; /* ns, WRITE Recovery Time defined by register MR */
	uint32_t tWL; /* tCK, Write Latency */
	uint32_t tRTP; /* tCK, READ to PRECHARGE command period */
	uint32_t tWTR; /* tCK, WRITE to READ command delay */

	/* DDR controller timing2 register */
	uint32_t tRL; /* tCK, Read Latency */
	uint32_t tRAS; /* ns, ACTIVE to PRECHARGE command period
                      to the same bank */
	uint32_t tRCD; /* ns, ACTIVE to READ or WRITE command period
                      to the same bank */
	uint32_t tCCD; /* tCK, CAS# to CAS# command delay (2 * tWTR) */

	/* DDR controller timing3 register */
	uint32_t tRP; /* ns, PRECHARGE command period to the same bank */
	uint32_t tRC; /* ns, ACTIVE to ACTIVE
                     command period to the same bank */
	uint32_t tRRD; /* ns, ACTIVE bank A to ACTIVE bank B command period */
	uint32_t tCKSRE; /** ns, Valid Clock Requirement after
                         Self Refresh Entry or Power-Down Entry */

	/* DDR controller timing4 register */
	uint32_t tCKE; /* tCK, CKE minimum pulse width */
	uint32_t tRFC; /* ns, AUTO-REFRESH command period */
	uint32_t tMINSR; /* Minimum Self-Refresh / Deep-Power-Down */
	uint32_t tXP; /* tCK, EXIT-POWER-DOWN to next valid command period */
	uint32_t tMRD; /* tCK, unit: tCK Load-Mode-Register
                     to next valid command period */

	/* DDR controller timing5 register */
	uint32_t tRDLAT;
	uint32_t tWDLAT;
	uint32_t tRTW; /* Read to Write delay */

	/* DDR controller timing6 register */
	uint32_t tFAW; /* ns, Four bank activate period */
	uint32_t tXS; /* ns, Exit self-refresh to next valid command delay */
	uint32_t tXSRD; /* ns, Exit self refresh to a read command */

	struct tck tck;
	struct size size;
} params;

struct jz4780_dram_params cpu_params = {
	.freq = 400000000,
	.ddr_odt = 1,
	.phy_odt = 1,
	.dq_odt = 1,
	.dqs_odt = 1,
	.pu_ohm = 0,
	.pd_ohm = 0,
	.cs0 = 1,
	.cs1 = 0,
	.dw32 = 1,
	.dll = 1,
	.div = 1,
	.col = 10,
	.row = 15,
	.bank8 = 1,
};

struct jz4775_dram_params cpu_paramsx = {
	.freq = 102000000,
	.ddr_odt = 0,
	.phy_odt = 0,
	.dq_odt = 0,
	.dqs_odt = 0,
	.pu_ohm = 0,
	.pd_ohm = 0,
	.cs0 = 1,
	.cs1 = 1,
	.dw32 = 1,
	.dll = 0,
	.div = 1,
	.col = 10,
	.row = 14,
	.bank8 = 0,
};

struct lpddr_params ddr_paramsx = {
	.bl = 4,
	.tCL = 3,
	.tCWL = 0,
	.tRAS = 40,
	.tRP = 15,
	.tRCD = 15,
	.tWL = 1,
	.tWR = 15,
	.tRRD = 10,
	.tRTP = 2,
	.tWTR = 2,
	.tRFC = 90,
	.tXP = 1,
	.tMRD = 2,
	.tCCD = 4,
	.tFAW = 50,
	.tCKE = 1,
	.tCKSRE = 1,
	.tDLLLOCK = 5120,
	.tMOD = 12,
	.tXPDLL = 10,
	.tXS = 140,
	.tXSRD = 100,
	.tREFI = 7800,
	.tDLLSRST = 1,
	.tAL = 0,
	.tMRD = 2,
};

struct ddr3_params ddr_params = {
	.bl = 8,
	.tCL = 7,
	.tCWL = 6,
	.tRAS = 38,
	.tRP = 14,
	.tRCD = 14,
	.tRC = 51,
	.tWR = 15,
	.tRRD = 4,
	.tRTP = 4,
	.tWTR = 4,
	.tRFC = 160,
	.tMINSR = 60,
	.tXP = 3,
	.tMRD = 4,
	.tCCD = 4,
	.tFAW = 50,
	.tCKE = 3,
	.tCKSRE = 5,
	.tDLLLOCK = 512,
	.tMOD = 12,
	.tXPDLL = 10,
	.tXS = 5,
	.tXSRD = 100,
	.tREFI = 7800,
	.tDLLSRST = 1,
};

static inline int calc_nck(int x_ns, int ps)
{
	int value;

	value = (x_ns * 1000) % ps == 0 ? (x_ns * 1000) / ps :
		(x_ns * 1000) / ps + 1;

	return value;
}

static uint32_t tck2ns(uint32_t ps, uint32_t tck, uint32_t time_ps)
{
	uint32_t value;
	value = (tck * ps > time_ps) ? (tck * ps) : time_ps;
	value = (value % 1000 == 0) ? (value / 1000) : (value / 1000 + 1);
	return value;
}

static void calculate_tck(uint32_t freq, struct tck *t)
{
	t->ps = (1000000000 / (freq / 1000));
	t->ns = (1000000000 % freq == 0) ?
		(1000000000 / freq) : (1000000000 / freq + 1);
}

static void calculate_size(struct ddr_params *p)
{
	uint32_t dw = p->dw32 ? 4 : 2;
	uint32_t banks = p->bank8 ? 8 : 4;
	uint32_t row = p->row;
	uint32_t col = p->col;

	if (p->cs0 == 1)
		p->size.chip0 =  (1 << (row + col)) * dw * banks;

	if (p->cs1 == 1)
		p->size.chip1 =  (1 << (row + col)) * dw * banks;
}

static void fill_dwc_params(int cpu_type, int ddr_type,
		void *cpu_struct, void *ddr_struct)
{
	uint32_t ps;

	params.ddr_type = ddr_type;
	params.cpu_type = cpu_type;

	switch (cpu_type) {
	case CPU_TYPE_JZ4780: {
		  struct jz4780_dram_params *p =
			  (struct jz4780_dram_params *)cpu_struct;
		  params.freq = p->freq;
		  params.ddr_odt = p->ddr_odt;
		  params.phy_odt = p->phy_odt;
		  params.dq_odt = p->dq_odt;
		  params.dqs_odt = p->dqs_odt;
		  params.pu_ohm = p->pu_ohm;
		  params.pd_ohm = p->pd_ohm;

		  params.cs0 = p->cs0;
		  params.cs1 = p->cs1;
		  params.dw32 = p->dw32;
		  params.dll = p->dll;
		  params.div = p->div;
		  params.col = p->col;
		  params.row = p->row;
		  params.bank8 = p->bank8;
		}
		break;
	case CPU_TYPE_JZ4775: {
		  struct jz4775_dram_params *p =
			  (struct jz4775_dram_params *)cpu_struct;
		  params.freq = p->freq;
		  params.ddr_odt = p->ddr_odt;
		  params.phy_odt = p->phy_odt;
		  params.dq_odt = p->dq_odt;
		  params.dqs_odt = p->dqs_odt;
		  params.pu_ohm = p->pu_ohm;
		  params.pd_ohm = p->pd_ohm;

		  params.cs0 = p->cs0;
		  params.cs1 = p->cs1;
		  params.dw32 = p->dw32;
		  params.dll = p->dll;
		  params.div = p->div;
		  params.col = p->col;
		  params.row = p->row;
		  params.bank8 = p->bank8;
		}
		break;
	default:
		return;
	}

	calculate_tck(params.freq, &params.tck);

	calculate_size(&params);

	ps = params.tck.ps;

	switch (ddr_type) {
	case DDR_TYPE_DDR3: {
		struct ddr3_params *p =
			(struct ddr3_params *)ddr_struct;
		params.bl = p->bl;
		params.tCL = p->tCL;
		params.tCWL = p->tCWL;
		params.tRAS = p->tRAS;
		params.tRP = p->tRP;
		params.tRCD = p->tRCD;
		params.tRC = p->tRC;
		params.tWR = p->tWR;
		params.tRRD = p->tRRD;
		params.tRTP = p->tRTP;
		params.tWTR = p->tWTR;
		params.tRFC = p->tRFC;
		params.tMINSR = p->tMINSR;
		params.tXP = p->tXP;
		params.tMRD = p->tMRD;
		params.tCCD = p->tCCD;
		params.tFAW = p->tFAW;
		params.tCKE = p->tCKE;
		params.tCKSRE = p->tCKSRE;
		params.tDLLLOCK = p->tDLLLOCK;
		params.tMOD = p->tMOD;
		params.tXPDLL = p->tXPDLL;
		params.tXS = p->tXS;
		params.tXSRD = p->tXSRD;
		params.tREFI = p->tREFI;
		params.tDLLSRST = p->tDLLSRST;

		if (params.dll == 0) {
			params.tCL = 6;
			params.tCWL = 6;
		}

		params.tRTP = tck2ns(ps, params.tRTP, 7500);
		params.tWTR = tck2ns(ps, params.tWTR, 7500);
		params.tRRD = tck2ns(ps, params.tRRD, 7500);
		params.tCKE = tck2ns(ps, params.tCKE, 5625);
		params.tRL = params.tAL + params.tCL;
		params.tWL = params.tAL + params.tCWL;
		params.tRDLAT = params.tRL - 2;
		params.tWDLAT = params.tWL - 1;
		params.tRTW = params.tRL + params.tCCD + 2 - params.tWL;
		params.tCKSRE = tck2ns(ps, params.tCKSRE, 10000);

		params.tXSDLL = tck2ns(ps, params.tDLLLOCK, 0);
		params.tMOD = tck2ns(ps, params.tMOD, 15 * 1000);
		params.tXPDLL = tck2ns(ps, params.tXPDLL, 24 * 1000);
		params.tXS = tck2ns(ps, params.tXS, (params.tRFC + 10) * 1000);

		params.tDLLSRST = 50; /* default 50ns */
		}
		break;
	case DDR_TYPE_LPDDR: {
		struct lpddr_params *p =
			(struct lpddr_params *)ddr_struct;
		params.bl = p->bl;
		params.tCL = p->tCL;
		params.tCWL = p->tCWL;
		params.tRAS = p->tRAS;
		params.tRP = p->tRP;
		params.tRCD = p->tRCD;
		params.tWR = p->tWR;
		params.tRRD = p->tRRD;
		params.tRTP = p->tRTP;
		params.tWTR = p->tWTR;
		params.tRFC = p->tRFC;
		params.tXP = p->tXP;
		params.tMRD = p->tMRD;
		params.tCCD = p->tCCD;
		params.tFAW = p->tFAW;
		params.tCKE = p->tCKE;
		params.tCKSRE = p->tCKSRE;
		params.tDLLLOCK = p->tDLLLOCK;
		params.tMOD = p->tMOD;
		params.tXPDLL = p->tXPDLL;
		params.tXS = p->tXS;
		params.tREFI = p->tREFI;
		params.tDLLSRST = p->tDLLSRST;
		params.tRRD = p->tRRD;
		params.tWL = p->tWL;

		params.tXSRD = calc_nck(params.tXS, ps);
		params.tMINSR = calc_nck(params.tRFC, ps);
		params.tRC = params.tRAS + params.tRP;
		params.tRTP = tck2ns(ps, params.tRTP, 0);
		params.tWTR = tck2ns(ps, params.tWTR, 0);
		params.tCKE = tck2ns(ps, params.tCKE, 0);
		params.tRL = params.tAL + params.tCL;
		params.tRDLAT = params.tRL - 2;
		params.tWDLAT = params.tWL - 1;
		params.tRTW = params.tRL + params.tCCD + 2 - params.tWL;
		params.tCKSRE = tck2ns(ps, params.tCKSRE, 0);

		params.tMOD = tck2ns(ps, params.tMOD, 15 * 1000);
		params.tXPDLL = tck2ns(ps, params.tXPDLL, 24 * 1000);

		params.tDLLSRST = 50; /* default 50ns */
		}
		break;
	default:
		return;
	}
}

static int fill_all_params(int cpu_type, int ddr_type,
		void *cpu_struct, void *ddr_struct)
{
	switch (cpu_type) {
	case CPU_TYPE_JZ4780:
		switch (ddr_type) {
		case DDR_TYPE_DDR3:
			fill_dwc_params(cpu_type, ddr_type,
				cpu_struct, ddr_struct);
			break;
		default:
			ddr_err("Soc4780 unsupported dram type %d\n", ddr_type);
			return -1;
		}
		break;
	case CPU_TYPE_JZ4775:
		switch (ddr_type) {
		case DDR_TYPE_LPDDR:
			fill_dwc_params(cpu_type, ddr_type,
				cpu_struct, ddr_struct);
			break;
		default:
			ddr_err("Soc4775 unsupported dram type %d\n", ddr_type);
			return -1;
		}
		break;
	default:
		ddr_err("Unsupported cpu type %d\n", ddr_type);
		return -1;
	}

	return 0;
}

static void dwc_ddrc_config(struct dwc_ddr_config *d, struct ddr_params *p)
{
	uint32_t tmp = 0, mem_base0 = 0, mem_base1 = 0, mem_mask0 = 0, mem_mask1 = 0;
	uint32_t memsize_cs0, memsize_cs1, memsize;
	struct tck *tck = &p->tck;

	struct ddrc_reg ddrc;
	memset(&ddrc, 0, sizeof(struct ddrc_reg));

	/* TIMING1,2,3,4,5,6 */
	ddrc.timing1.b.tRTP = calc_nck(p->tRTP, tck->ps);
	ddrc.timing1.b.tWTR = calc_nck(p->tWTR, tck->ps) + p->tWL + p->bl / 2; //??
	ddrc.timing1.b.tWR = calc_nck(p->tWR, tck->ps);
	if (p->ddr_type == DDR_TYPE_DDR3)
		BETWEEN(ddrc.timing1.b.tWR, 5, 12);
	else
		BETWEEN(ddrc.timing1.b.tWR, 2, 6);
	ddrc.timing1.b.tWL = p->tWL;

//	printf("ps %d rtp %d wtr %d wr %d wl %d %x", tck->ps, p->tRTP,
//			p->tWTR, p->tWR, p->tWL, ddrc.timing1.d32);

	ddrc.timing2.b.tCCD = p->tCCD;
	ddrc.timing2.b.tRAS = calc_nck(p->tRAS, tck->ps);
	ddrc.timing2.b.tRCD = calc_nck(p->tRCD, tck->ps);
	ddrc.timing2.b.tRL = p->tRL;

	/* Keep to 4 in this version */
	ddrc.timing3.b.ONUM = 4;

	if (p->ddr_type != DDR_TYPE_DDR3) {
		u32 tmp = calc_nck(p->tCKSRE, tck->ps) / 8;
		BETWEEN(tmp, 1, 7);
		ddrc.timing3.b.tCKSRE = tmp;
	} else
		/* Set DDR_tCKSRE to max to ensafe suspend & resume */
		ddrc.timing3.b.tCKSRE = 7;

	ddrc.timing3.b.tRP = calc_nck(p->tRP, tck->ps);
	ddrc.timing3.b.tRRD = calc_nck(p->tRRD, tck->ps);
	ddrc.timing3.b.tRC = calc_nck(p->tRC, tck->ps);

	ddrc.timing4.b.tRFC = (calc_nck(p->tRFC, tck->ps) - 1) / 2;
	ddrc.timing4.b.tEXTRW = 3;/* Keep to 3 in this version */
	ddrc.timing4.b.tRWCOV = 3;/* Keep to 3 in this version */
	ddrc.timing4.b.tCKE = calc_nck(p->tCKE, tck->ps);
	tmp = p->tMINSR;
	BETWEEN(tmp, 9, 129);
	tmp = ((tmp - 1) % 8) ? ((tmp - 1) / 8) : ((tmp - 1) / 8 - 1);
	ddrc.timing4.b.tMINSR = tmp;
	ddrc.timing4.b.tXP = p->tXP;
	ddrc.timing4.b.tMRD = p->tMRD - 1;

	ddrc.timing5.b.tCTLUPD = 0xff; /* 0xff is the default value */
	ddrc.timing5.b.tRTW = p->tRTW;
	ddrc.timing5.b.tRDLAT = p->tRDLAT;
	ddrc.timing5.b.tWDLAT = p->tWDLAT;

	ddrc.timing6.b.tXSRD = p->tXSRD / 4;
	ddrc.timing6.b.tFAW = calc_nck(p->tFAW, tck->ps);
	ddrc.timing6.b.tCFGW = 2;
	ddrc.timing6.b.tCFGR = 2;

	/* REFCNT */
	tmp = p->tREFI / tck->ns;
	tmp = tmp / (16 * (1 << p->div)) - 1;
	BETWEEN(tmp, 1, 0xff);
	ddrc.refcnt = (tmp << DDRC_REFCNT_CON_BIT)
		| (p->div << DDRC_REFCNT_CLK_DIV_BIT)
		| DDRC_REFCNT_REF_EN;

	/* CFG */
	ddrc.cfg.b.ROW1 = p->row - 12;
	ddrc.cfg.b.COL1 = p->col - 8;
	ddrc.cfg.b.BA1 = p->bank8;
	ddrc.cfg.b.IMBA = 1;
	ddrc.cfg.b.BSL = (p->bl == 8) ? 1 : 0;
	if (p->ddr_odt)
		ddrc.cfg.b.ODTEN = 1;
	else
		ddrc.cfg.b.ODTEN = 0;
	ddrc.cfg.b.MISPE = 1;
	ddrc.cfg.b.ROW0 = p->row - 12;
	ddrc.cfg.b.COL0 = p->col - 8;
	ddrc.cfg.b.CS1EN = p->cs1;
	ddrc.cfg.b.CS0EN = p->cs0;
	ddrc.cfg.b.CL = 0; /* NOT used in this version */
	ddrc.cfg.b.BA0 = p->bank8;
	ddrc.cfg.b.DW = p->dw32;
	switch (p->ddr_type) {
#define _CASE(D, P)    \
	case D:            \
		ddrc.cfg.b.TYPE = P; \
		break
		_CASE(DDR_TYPE_DDR3, 6);    /* DDR3:0b110 */
		_CASE(DDR_TYPE_LPDDR, 3);   /* LPDDR:0b011 */
		_CASE(DDR_TYPE_LPDDR2, 5);  /* LPDDR2:0b101 */
#undef _CASE
	default:
		break;
	}
	/* CTRL */
	switch (p->ddr_type) {
	case DDR_TYPE_DDR3:
		ddrc.ctrl = DDRC_CTRL_ACTPD | DDRC_CTRL_PDT_64 | DDRC_CTRL_ACTSTP
			| DDRC_CTRL_PRET_8 | 0 << 6 | DDRC_CTRL_UNALIGN
			| DDRC_CTRL_ALH | DDRC_CTRL_RDC | DDRC_CTRL_CKE;
		break;
	case DDR_TYPE_LPDDR:
		ddrc.ctrl = DDRC_CTRL_PDT_64 | DDRC_CTRL_ACTSTP | DDRC_CTRL_PRET_8
			| 0 << 6 | DDRC_CTRL_UNALIGN | DDRC_CTRL_ALH | DDRC_CTRL_RDC
			| DDRC_CTRL_CKE;
		break;
	}

	/* MMAP0,1 */
	memsize_cs0 = p->size.chip0;
	memsize_cs1 = p->size.chip1;
	memsize = memsize_cs0 + memsize_cs1;

	if (memsize > 0x20000000) {
		if (memsize_cs1) {
			mem_base0 = 0x0;
			mem_mask0 = (~((memsize_cs0 >> 24) - 1) & ~(memsize >> 24))
				& DDRC_MMAP_MASK_MASK;
			mem_base1 = (memsize_cs1 >> 24) & 0xff;
			mem_mask1 = (~((memsize_cs1 >> 24) - 1) & ~(memsize >> 24))
				& DDRC_MMAP_MASK_MASK;
		} else {
			mem_base0 = 0x0;
			mem_mask0 = ~(((memsize_cs0 * 2) >> 24) - 1) & DDRC_MMAP_MASK_MASK;
			mem_mask1 = 0;
			mem_base1 = 0xff;
		}
	} else {
		mem_base0 = (DDR_MEM_PHY_BASE >> 24) & 0xff;
		mem_mask0 = ~((memsize_cs0 >> 24) - 1) & DDRC_MMAP_MASK_MASK;
		mem_base1 = ((DDR_MEM_PHY_BASE + memsize_cs0) >> 24) & 0xff;
		mem_mask1 = ~((memsize_cs1 >> 24) - 1) & DDRC_MMAP_MASK_MASK;
	}
	ddrc.mmap[0] = mem_base0 << DDRC_MMAP_BASE_BIT | mem_mask0;
	ddrc.mmap[1] = mem_base1 << DDRC_MMAP_BASE_BIT | mem_mask1;

	d->ddrc_cfg = ddrc.cfg.d32;
	d->ddrc_ctrl = ddrc.ctrl;
	d->ddrc_refcnt = ddrc.refcnt;
	d->ddrc_mmap0 = ddrc.mmap[0];
	d->ddrc_mmap1 = ddrc.mmap[1];
	d->ddrc_timing1 = ddrc.timing1.d32;
	d->ddrc_timing2 = ddrc.timing2.d32;
	d->ddrc_timing3 = ddrc.timing3.d32;
	d->ddrc_timing4 = ddrc.timing4.d32;
	d->ddrc_timing5 = ddrc.timing5.d32;
	d->ddrc_timing6 = ddrc.timing6.d32;
}

static void dwc_ddrp_config(struct dwc_ddr_config *d, struct ddr_params *p)
{
	uint32_t tmp = 0;
	uint32_t dinit1 = 0;
	struct tck *tck = &p->tck;

	struct ddrp_reg ddrp;
	memset(&ddrp, 0, sizeof(struct ddrp_reg));

#define PNDEF(N, P, T, MIN, MAX, PS)   \
		T = calc_nck(p->P, PS);        \
		BETWEEN(T, MIN, MAX);          \
		ddrp.dtpr##N.b.P = T

	switch (p->ddr_type) {
	case DDR_TYPE_DDR3:
		/* DCR register */
		ddrp.dcr = 3 | (p->bank8 << 3);

		/* MRn registers */
		tmp = calc_nck(p->tWR, tck->ps);
		BETWEEN(tmp, 5, 12);
		if (tmp < 8)
			tmp -= 4;
		else
			tmp = (tmp + 1) / 2;
		ddrp.mr0.ddr3.WR = tmp;
		ddrp.mr0.ddr3.CL_4_6 = p->tCL - 4;
		ddrp.mr0.ddr3.BL = (8 - p->bl) / 2;
		ddrp.mr1.ddr3.DIC1 = 1; /* Impedance=RZQ/7 */
		if (p->ddr_odt)
			ddrp.mr1.ddr3.RTT2 = 1; /* Effective resistance of ODT RZQ/4 */

		if (!p->dll)
			ddrp.mr1.ddr3.DE = 1; /* DLL disable */

		ddrp.mr2.ddr3.CWL = p->tCWL - 5;

		/* PTRn registers */
		ddrp.ptr0.b.tDLLSRST = calc_nck(p->tDLLSRST, tck->ps);
		ddrp.ptr0.b.tDLLLOCK = calc_nck(5120, tck->ps); /* DDR3 default 5.12us*/
		ddrp.ptr0.b.tITMSRST = 8;

		ddrp.ptr1.b.tDINIT0 = calc_nck(500000, tck->ps); /* DDR3 default 500us*/
		if (((p->tRFC + 10) * 1000) > (5 * tck->ps))  /* ddr3 only */
			dinit1 = (p->tRFC + 10) * 1000;
		else
			dinit1 = 5 * tck->ps;
		tmp = calc_nck(dinit1 / 1000, tck->ps);
		ddrp.ptr1.b.tDINIT1 = tmp;
		if (tmp > 0xff)
			tmp = 0xff;
		ddrp.ptr2.b.tDINIT2 = calc_nck(200000, tck->ps); /* DDR3 default 200us*/
		ddrp.ptr2.b.tDINIT3 = 512;

		/* DTPR0 registers */
		ddrp.dtpr0.b.tMRD = p->tMRD;
		PNDEF(0, tRTP, tmp, 2, 6, tck->ps);
		PNDEF(0, tWTR, tmp, 1, 6, tck->ps);
		PNDEF(0, tRP, tmp, 2, 11, tck->ps);
		PNDEF(0, tRCD, tmp, 2, 11, tck->ps);
		PNDEF(0, tRAS, tmp, 2, 31, tck->ps);
		PNDEF(0, tRRD, tmp, 1, 8, tck->ps);
		PNDEF(0, tRC, tmp, 2, 42, tck->ps);
		ddrp.dtpr0.b.tCCD = (p->tCCD > 4) ? 1 : 0;

		/* DTPR1 registers */
		PNDEF(1, tFAW, tmp, 2, 31, tck->ps);
		PNDEF(1, tMOD, tmp, 12, 15, tck->ps);
		ddrp.dtpr1.b.tMOD -= 12;
		PNDEF(1, tRFC, tmp, 1, 255, tck->ps);
		ddrp.dtpr1.b.tRTODT = 1;

		/* DTPR2 registers */
		tmp = (p->tXS > p->tXSDLL) ? p->tXS : p->tXSDLL;
		tmp = calc_nck(tmp, tck->ps);
		BETWEEN(tmp, 2, 1023);
		ddrp.dtpr2.b.tXS = tmp;

		tmp = (p->tXP > p->tXPDLL) ? p->tXP : p->tXPDLL;
		tmp = calc_nck(tmp, tck->ps);
		BETWEEN(tmp, 2, 31);
		ddrp.dtpr2.b.tXP = tmp;

		tmp = p->tCKE;
		BETWEEN(tmp, 2, 15);
		ddrp.dtpr2.b.tCKE = tmp;

		tmp = p->tDLLLOCK;
		BETWEEN(tmp, 2, 1023);
		ddrp.dtpr2.b.tDLLK = tmp;

		/* PGCR registers */
		ddrp.pgcr = DDRP_PGCR_DQSCFG | 7 << DDRP_PGCR_CKEN_BIT
			| 2 << DDRP_PGCR_CKDV_BIT
			| (p->cs0 | p->cs1 << 1) << DDRP_PGCR_RANKEN_BIT
			| DDRP_PGCR_ZCKSEL_32 | DDRP_PGCR_PDDISDX;
		break;
	case DDR_TYPE_LPDDR:
		/* DCR register */
		ddrp.dcr = 0 | (p->bank8 << 3);
		/* MRn registers */
		tmp = 1;
		while (p->bl >> tmp)
			tmp++;
		ddrp.mr0.lpddr.BL = tmp;
		ddrp.mr0.lpddr.CL = p->tCL;
		/* PTRn registers */
		ddrp.ptr0.b.tDLLSRST = calc_nck(50, tck->ps);
		/* LPDDR default 5.12us*/
		ddrp.ptr0.b.tDLLLOCK = calc_nck(5120, tck->ps);
		ddrp.ptr0.b.tITMSRST = 8;
		/* tDINIT0 LPDDR default 200us*/
		ddrp.ptr1.b.tDINIT0 = calc_nck(200000, tck->ps);
		ddrp.ptr1.b.tDINIT1 = calc_nck(100, tck->ps);

		ddrp.ptr2.b.tDINIT2 = calc_nck(100, tck->ps);
		ddrp.ptr2.b.tDINIT3 = calc_nck(100, tck->ps);

		/* DTPR0 registers */
		ddrp.dtpr0.b.tMRD = p->tMRD;
		PNDEF(0, tRTP, tmp, 2, 6, tck->ps);
		PNDEF(0, tWTR, tmp, 1, 6, tck->ps);
		PNDEF(0, tRP, tmp, 2, 11, tck->ps);
		PNDEF(0, tRCD, tmp, 2, 11, tck->ps);
		PNDEF(0, tRAS, tmp, 2, 31, tck->ps);
		PNDEF(0, tRRD, tmp, 1, 8, tck->ps);
		PNDEF(0, tRC, tmp, 2, 42, tck->ps);
		ddrp.dtpr0.b.tCCD = (p->tCCD > (p->bl / 2)) ? 1 : 0;
		/* DTPR1 registers */
		PNDEF(1, tFAW, tmp, 2, 31, tck->ps);
		PNDEF(1, tRFC, tmp, 1, 255, tck->ps);

		/* DTPR2 registers */
		tmp = calc_nck(p->tXS, tck->ps);
		BETWEEN(tmp, 2, 0x3ff);
		ddrp.dtpr2.b.tXS = tmp;

		tmp = calc_nck(p->tXP, tck->ps);
		BETWEEN(tmp, 2, 0x1f);
		ddrp.dtpr2.b.tXP = tmp;

		tmp = p->tCKE;
		BETWEEN(tmp, 2, 15);
		ddrp.dtpr2.b.tCKE = tmp;

		tmp = calc_nck(p->tDLLLOCK, tck->ps);
		BETWEEN(tmp, 2, 1023);
		ddrp.dtpr2.b.tDLLK = tmp;

		/* PGCR registers */
		ddrp.pgcr = DDRP_PGCR_ITMDMD | DDRP_PGCR_DQSCFG
			| 7 << DDRP_PGCR_CKEN_BIT | 2 << DDRP_PGCR_CKDV_BIT
			| (p->cs0 | p->cs1 << 1) << DDRP_PGCR_RANKEN_BIT
			| DDRP_PGCR_PDDISDX;
		break;
	case DDR_TYPE_LPDDR2:
		ddrp.dcr = 4 | (p->bank8 << 3);
		break;
	default:
		break;
	}
#undef PNDEF

	d->ddrp_dcr = ddrp.dcr;
	d->ddrp_mr0 = ddrp.mr0.d32;
	d->ddrp_mr1 = ddrp.mr1.d32;
	d->ddrp_mr2 = ddrp.mr2.d32;
	d->ddrp_mr3 = ddrp.mr3.d32;

	d->ddrp_odtcr = ddrp.odtcr;
	d->ddrp_pgcr = ddrp.pgcr;

	d->ddrp_ptr0 = ddrp.ptr0.d32;
	d->ddrp_ptr1 = ddrp.ptr1.d32;
	d->ddrp_ptr2 = ddrp.ptr2.d32;
	d->ddrp_dtpr0 = ddrp.dtpr0.d32;
	d->ddrp_dtpr1 = ddrp.dtpr1.d32;
	d->ddrp_dtpr2 = ddrp.dtpr2.d32;
}

static void params_print(struct dwc_ddr_config *d)
{
	/* DDRC registers print */
	printf("#define DDRC_CFG_VALUE			0x%08x\n", d->ddrc_cfg);
	printf("#define DDRC_CTRL_VALUE			0x%08x\n", d->ddrc_ctrl);
	printf("#define DDRC_refcnt_VALUE		0x%08x\n", d->ddrc_refcnt);
	printf("#define DDRC_MMAP0_VALUE		0x%08x\n", d->ddrc_mmap0);
	printf("#define DDRC_MMAP1_VALUE		0x%08x\n", d->ddrc_mmap1);
	printf("#define DDRC_TIMING1_VALUE		0x%08x\n", d->ddrc_timing1);
	printf("#define DDRC_TIMING2_VALUE		0x%08x\n", d->ddrc_timing2);
	printf("#define DDRC_TIMING3_VALUE		0x%08x\n", d->ddrc_timing3);
	printf("#define DDRC_TIMING4_VALUE		0x%08x\n", d->ddrc_timing4);
	printf("#define DDRC_TIMING5_VALUE		0x%08x\n", d->ddrc_timing5);
	printf("#define DDRC_TIMING6_VALUE		0x%08x\n", d->ddrc_timing6);

	/* DDRP registers print */
	printf("#define DDRP_DCR_VALUE			0x%08x\n", d->ddrp_dcr);
	printf("#define DDRP_MR0_VALUE			0x%08x\n", d->ddrp_mr0);
	printf("#define DDRP_MR1_VALUE			0x%08x\n", d->ddrp_mr1);
	printf("#define DDRP_MR2_VALUE			0x%08x\n", d->ddrp_mr2);
	printf("#define DDRP_PTR0_VALUE			0x%08x\n", d->ddrp_ptr0);
	printf("#define DDRP_PTR1_VALUE			0x%08x\n", d->ddrp_ptr1);
	printf("#define DDRP_PTR2_VALUE			0x%08x\n", d->ddrp_ptr2);
	printf("#define DDRP_DTPR0_VALUE		0x%08x\n", d->ddrp_dtpr0);
	printf("#define DDRP_DTPR1_VALUE		0x%08x\n", d->ddrp_dtpr1);
	printf("#define DDRP_DTPR2_VALUE		0x%08x\n", d->ddrp_dtpr2);
	printf("#define DDRP_PGCR_VALUE			0x%08x\n", d->ddrp_pgcr);

	printf("#define DDRP_odt_VALUE			0x%08x\n", d->ddr_odt);
	printf("#define DDRP_size0_VALUE		%uMB\n", d->ddr_size0/(1024 * 1024));
	printf("#define DDRP_size1_VALUE		%uMB\n", d->ddr_size1/(1024 * 1024));
}

static void *create_dwc_config(struct ddr_params *p)
{
	struct dwc_ddr_config *d;

	d = (struct dwc_ddr_config *)malloc(sizeof(struct dwc_ddr_config));
	memset(d, 0, sizeof(struct dwc_ddr_config));

	dwc_ddrc_config(d, p);

	dwc_ddrp_config(d, p);

	d->ddr_odt = (p->ddr_odt << DDR_ODT_BIT)
		| (p->phy_odt << PHY_ODT_BIT) | (p->dq_odt << DQ_ODT_BIT)
		| (p->dqs_odt << DQS_ODT_BIT) | (p->pu_ohm << PU_ODT_OFF)
		| (p->pd_ohm << PD_ODT_OFF);

	d->ddr_size0 = p->size.chip0;
	d->ddr_size1 = p->size.chip1;

	params_print(d);

	return (void *)d;
}

static void *create_ddr_config(int cpu_type, int ddr_type,
		void *cpu_struct, void *ddr_struct)
{
	switch (cpu_type) {
	case CPU_TYPE_JZ4780:
	case CPU_TYPE_JZ4775:
		return create_dwc_config(&params);
		break;
	default:
		break;
	}

	return NULL;
}

void *ddr_params_creator(int cpu_type, int ddr_type,
		void *cpu_struct, void *ddr_struct)
{
	int ret;
	void *p;

	ret = fill_all_params(cpu_type, ddr_type,
			cpu_struct, ddr_struct);
	if (ret) {
		ddr_err("dram fill all parameters error!\n");
		return NULL;
	}

	p = create_ddr_config(cpu_type, ddr_type,
			cpu_struct, ddr_struct);
	if (p == NULL) {
		ddr_err("dram create dram configurate error!\n");
		return NULL;
	}
	return p;
}

#if 1
int main(int argc, char *argv[])
{
//	ddr_params_creator(CPU_TYPE_JZ4780, DDR_TYPE_DDR3,
//			&cpu_params, &ddr_params);
	ddr_params_creator(CPU_TYPE_JZ4775, DDR_TYPE_LPDDR,
			&cpu_paramsx, &ddr_paramsx);
	return 0;
}
#endif
