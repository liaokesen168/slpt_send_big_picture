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

#ifndef __DDR_REGISTER_H__
#define __DDR_REGISTER_H__

typedef union ddrp_mr0 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned BL:2;
		unsigned CL_2:1;
		unsigned BT:1;
		unsigned CL_4_6:3;
		unsigned TM:1;
		unsigned DR:1;
		unsigned WR:3;
		unsigned PD:1;
		unsigned RSVD:3;
		unsigned reserved16_31:16;
	} ddr3; /* MR0 */
	struct {
		unsigned BL:3;
		unsigned BT:1;
		unsigned CL:3;
		unsigned TM:1;
		unsigned RSVD8_11:4;
		unsigned RSVD12_15:4;
		unsigned reserved16_31:16;
	} lpddr; /* MR */
	struct {
		unsigned unsupported:31;
	} ddr2; /* MR */
	struct {
		unsigned unsupported:31;
	} ddr; /* MR */
} ddrp_mr0_t;

typedef union ddrp_mr1 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned DE:1;
		unsigned DIC1:1;
		unsigned RTT2:1;
		unsigned AL:2;
		unsigned DIC5:1;
		unsigned RTT6:1;
		unsigned LEVEL:1;
		unsigned RSVD8:1;
		unsigned RTT9:1;
		unsigned RSVD10:1;
		unsigned TDQS:1;
		unsigned QOFF:1;
		unsigned RSVD13_15:3;
		unsigned reserved16_31:16;
	} ddr3; /* MR1 */
	struct {
		unsigned BL:3;
		unsigned BT:1;
		unsigned WC:1;
		unsigned nWR:3;
		unsigned reserved8_31:24;
	} lpddr2; /* MR1 */
	struct {
		unsigned unsupported:31;
	} ddr2; /* EMR */
	struct {
		unsigned unsupported:31;
	} ddr; /* EMR */
} ddrp_mr1_t;

typedef union ddrp_mr2 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned PASR:3;
		unsigned CWL:3;
		unsigned ASR:1;
		unsigned SRT:1;
		unsigned RSVD8:1;
		unsigned RTTWR:2;
		unsigned RSVD11_15:5;
		unsigned reserved16_31:16;
	} ddr3; /* MR2 */
	struct {
		unsigned PASR:3;
		unsigned TCSR:2;
		unsigned DS:3;
		unsigned RSVD8_15:8;
		unsigned reserved16_31:16;
	} lpddr; /* EMR */
	struct {
		unsigned RL_WL:4;
		unsigned RSVD4_7:4;
		unsigned reserved8_31:24;
	} lpddr2; /* MR2 */
	struct {
		unsigned unsupported:31;
	} ddr2; /* EMR2 */
} ddrp_mr2_t;

typedef union ddrp_mr3 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned MPRLOC:2;
		unsigned MPR:1;
		unsigned RSVD3_15:13;
		unsigned reserved16_31:16;
	} ddr3; /* MR3 */
	struct {
		unsigned DS:4;
		unsigned RSVD4_7:4;
		unsigned reserved8_31:24;
	} lpddr2; /* MR2 */
	struct {
		unsigned unsupported:31;
	} ddr2; /* EMR3 */
} ddrp_mr3_t;

typedef union ddrp_ptr0 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tDLLSRST:6;
		unsigned tDLLLOCK:12;
		unsigned tITMSRST:4;
		unsigned reserved22_31:10;
	} b;
} ddrp_ptr0_t;

typedef union ddrp_ptr1 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tDINIT0:19;
		unsigned tDINIT1:8;
		unsigned reserved27_31:5;
	} b;
} ddrp_ptr1_t;

typedef union ddrp_ptr2 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tDINIT2:17;
		unsigned tDINIT3:10;
		unsigned reserved27_31:5;
	} b;
} ddrp_ptr2_t;

typedef union ddrp_dtpr0 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tMRD:2;
		unsigned tRTP:3;
		unsigned tWTR:3;
		unsigned tRP:4;
		unsigned tRCD:4;
		unsigned tRAS:5;
		unsigned tRRD:4;
		unsigned tRC:6;
		unsigned tCCD:1;
	} b;
} ddrp_dtpr0_t;

typedef union ddrp_dtpr1 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tAOND_tAOFD:2;
		unsigned tRTW:1;
		unsigned tFAW:6;
		unsigned tMOD:2;
		unsigned tRTODT:1;
		unsigned reserved12_15:4;
		unsigned tRFC:8;
		unsigned tDQSCK:3;
		unsigned tDQSCKmax:3;
		unsigned reserved30_31:2;
	} b;
} ddrp_dtpr1_t;

typedef union ddrp_dtpr2 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tXS:10;
		unsigned tXP:5;
		unsigned tCKE:4;
		unsigned tDLLK:10;
		unsigned reserved29_31:3;
	} b;
} ddrp_dtpr2_t;

struct ddrp_reg {
	uint32_t dcr;
	ddrp_mr0_t mr0;
	ddrp_mr1_t mr1;
	ddrp_mr2_t mr2;
	ddrp_mr3_t mr3;
	uint32_t odtcr;
	uint32_t pgcr;
	ddrp_ptr0_t ptr0;
	ddrp_ptr1_t ptr1;
	ddrp_ptr2_t ptr2;
	ddrp_dtpr0_t dtpr0;
	ddrp_dtpr1_t dtpr1;
	ddrp_dtpr2_t dtpr2;
};

typedef union ddrc_timing1 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tWL:6;
		unsigned reserved6_7:2;
		unsigned tWR:6;
		unsigned reserved14_15:2;
		unsigned tWTR:6;
		unsigned reserved22_23:2;
		unsigned tRTP:6;
		unsigned reserved30_31:2;
	} b;
} ddrc_timing1_t;

typedef union ddrc_timing2 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tRL:6;
		unsigned reserved6_7:2;
		unsigned tRCD:6;
		unsigned reserved14_15:2;
		unsigned tRAS:6;
		unsigned reserved22_23:2;
		unsigned tCCD:6;
		unsigned reserved30_31:2;
	} b;
} ddrc_timing2_t;

typedef union ddrc_timing3 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tRC:6;
		unsigned reserved6_7:2;
		unsigned tRRD:6;
		unsigned reserved14_15:2;
		unsigned tRP:6;
		unsigned reserved22_23:2;
		unsigned tCKSRE:3;
		unsigned ONUM:4;
		unsigned reserved31:1;
	} b;
} ddrc_timing3_t;

typedef union ddrc_timing4 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tMRD:2;
		unsigned reserved2_3:2;
		unsigned tXP:3;
		unsigned reserved7:1;
		unsigned tMINSR:4;
		unsigned reserved12_15:4;
		unsigned tCKE:3;
		unsigned tRWCOV:2;
		unsigned tEXTRW:3;
		unsigned tRFC:6;
		unsigned reserved30_31:2;
	} b;
} ddrc_timing4_t;

typedef union ddrc_timing5 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tWDLAT:6;
		unsigned reserved6_7:2;
		unsigned tRDLAT:6;
		unsigned reserved14_15:2;
		unsigned tRTW:6;
		unsigned reserved22_23:2;
		unsigned tCTLUPD:8;
	} b;
} ddrc_timing5_t;

typedef union ddrc_timing6 {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned tCFGR:6;
		unsigned reserved6_7:2;
		unsigned tCFGW:6;
		unsigned reserved14_15:2;
		unsigned tFAW:6;
		unsigned reserved22_23:2;
		unsigned tXSRD:8;
	} b;
} ddrc_timing6_t;

typedef union ddrc_cfg {
	/** raw register data */
	uint32_t d32;
	/** register bits */
	struct {
		unsigned DW:1;
		unsigned BA0:1;
		unsigned CL:4;
		unsigned CS0EN:1;
		unsigned CS1EN:1;
		unsigned COL0:3;
		unsigned ROW0:3;
		unsigned reserved14:1;
		unsigned MISPE:1;
		unsigned ODTEN:1;
		unsigned TYPE:3;
		unsigned reserved20:1;
		unsigned BSL:1;
		unsigned IMBA:1;
		unsigned BA1:1;
		unsigned COL1:3;
		unsigned ROW1:3;
		unsigned reserved30_31:2;
	} b;
} ddrc_cfg_t;

struct ddrc_reg {
	ddrc_cfg_t cfg;
	uint32_t ctrl;
	uint32_t refcnt;
	uint32_t mmap[2];
	uint32_t remap[5];
	ddrc_timing1_t timing1;
	ddrc_timing2_t timing2;
	ddrc_timing3_t timing3;
	ddrc_timing4_t timing4;
	ddrc_timing5_t timing5;
	ddrc_timing6_t timing6;
};

#endif

