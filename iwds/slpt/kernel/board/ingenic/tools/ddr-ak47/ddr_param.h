/*
 * DDR parameters data structure.
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
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

#ifndef __DDR_PARAMS_H__
#define __DDR_PARAMS_H__

#include "../../spl/ioctl.h"
#include "../../spl/config.h"

#define CPU_TYPE_CODE 'C'
#define DDR_TYPE_CODE 'D'

#define CPU_TYPE_JZ4780 _IOW(CPU_TYPE_CODE, 4780, short)
#define CPU_TYPE_JZ4775 _IOW(CPU_TYPE_CODE, 4775, short)
#define CPU_TYPE_JZ4785 _IOW(CPU_TYPE_CODE, 4785, short)

#define DDR_TYPE_DDR3 _IOW(DDR_TYPE_CODE, 1, short)
#define DDR_TYPE_LPDDR _IOW(DDR_TYPE_CODE, 2, short)
#define DDR_TYPE_LPDDR2 _IOW(DDR_TYPE_CODE, 3, short)

struct jz4780_dram_params {
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
	uint32_t col; /* column address */
	uint32_t row; /* row address */
	uint32_t bank8; /* Banks each chip: 0: 4banks, 1: 8banks */
};

struct jz4775_dram_params {
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
	uint32_t col; /* column address */
	uint32_t row; /* row address */
	uint32_t bank8; /* Banks each chip: 0: 4banks, 1: 8banks */
};

struct ddr3_params {
	/* DDR configuration */
	uint32_t bl; /* Burst length */
	uint32_t tCL; /* CAS latency */

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
};

struct lpddr_params {
	/* DDR configuration */
	uint32_t bl; /* Burst length */
	uint32_t tCL; /* CAS latency */

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
};

struct dwc_ddr_config {
	/* DDRC registers print */
	uint32_t ddrc_cfg;
	uint32_t ddrc_ctrl;
	uint32_t ddrc_refcnt;
	uint32_t ddrc_mmap0;
	uint32_t ddrc_mmap1;
	uint32_t ddrc_timing1;
	uint32_t ddrc_timing2;
	uint32_t ddrc_timing3;
	uint32_t ddrc_timing4;
	uint32_t ddrc_timing5;
	uint32_t ddrc_timing6;

	/* DDRP registers print */
	uint32_t ddrp_dcr;
	uint32_t ddrp_mr0;
	uint32_t ddrp_mr1;
	uint32_t ddrp_mr2;
	uint32_t ddrp_mr3;
	uint32_t ddrp_ptr0;
	uint32_t ddrp_ptr1;
	uint32_t ddrp_ptr2;
	uint32_t ddrp_dtpr0;
	uint32_t ddrp_dtpr1;
	uint32_t ddrp_dtpr2;

	uint32_t ddrp_pgcr;
	uint32_t ddrp_odtcr;

	uint32_t ddr_odt;
	uint32_t ddr_size0;
	uint32_t ddr_size1;
};

#endif /* __DDR_PARAMS_H__ */

