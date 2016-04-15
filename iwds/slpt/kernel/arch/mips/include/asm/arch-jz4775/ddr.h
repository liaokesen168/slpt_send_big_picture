/*
 * Ingenic DDR Controller driver head file
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co., Ltd.
 *			Kage Shen <kkshen@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __JZ4780_DDR_H__
#define __JZ4780_DDR_H__

#define	DDRC_IOBASE			0xB3010000
#define DDR_MEM_PHY_BASE 	0x20000000
#define DDR_PHY_OFFSET		0x1000

#define	DDRC_STAT			0x000
#define	DDRC_CFG			0x004
#define	DDRC_CTRL			0x008
#define	DDRC_LMR			0x00c
#define	DDRC_TIMING(n)		(0x060 + (n - 1) * 4)
#define	DDRC_REFCNT			0x018
#define	DDRC_MMAP0			0x024
#define	DDRC_MMAP1			0x028
#define DDRC_LPC			0x0bc
#define DDRC_REMAP(n)		(0x09c + (n - 1) * 4)

#define	rDDRC_STAT			(0x000 + DDRC_IOBASE)
#define	rDDRC_CFG			(0x004 + DDRC_IOBASE)
#define	rDDRC_CTRL			(0x008 + DDRC_IOBASE)
#define	rDDRC_LMR			(0x00c + DDRC_IOBASE)
#define	rDDRC_TIMING(n)		(0x060 + (n - 1) * 4 + DDRC_IOBASE)
#define	rDDRC_REFCNT		(0x018 + DDRC_IOBASE)
#define	rDDRC_MMAP0			(0x024 + DDRC_IOBASE)
#define	rDDRC_MMAP1			(0x028 + DDRC_IOBASE)
#define rDDRC_LPC			(0x0bc + DDRC_IOBASE)
#define rDDRC_REMAP(n)		(0x09c + (n - 1) * 4 + DDRC_IOBASE)

#define rDDRP_PIR	(DDRC_IOBASE + DDR_PHY_OFFSET + 0x4) /* PHY Initialization Register */
#define rDDRP_PGCR	(DDRC_IOBASE + DDR_PHY_OFFSET + 0x8) /* PHY General Configuration Register*/
#define rDDRP_PGSR	(DDRC_IOBASE + DDR_PHY_OFFSET + 0xc) /* PHY General Status Register*/

#define rDDRP_PTR0	(DDRC_IOBASE + DDR_PHY_OFFSET + 0x18) /* PHY Timing Register 0 */
#define rDDRP_PTR1	(DDRC_IOBASE + DDR_PHY_OFFSET + 0x1c) /* PHY Timing Register 1 */
#define rDDRP_PTR2	(DDRC_IOBASE + DDR_PHY_OFFSET + 0x20) /* PHY Timing Register 2 */

#define rDDRP_ACIOCR	(DDRC_IOBASE + DDR_PHY_OFFSET + 0x24) /* AC I/O Configuration Register */
#define rDDRP_DXCCR		(DDRC_IOBASE + DDR_PHY_OFFSET + 0x28) /* DATX8 Common Configuration Register */
#define rDDRP_DSGCR		(DDRC_IOBASE + DDR_PHY_OFFSET + 0x2c) /* DDR System General Configuration Register */
#define rDDRP_DCR		(DDRC_IOBASE + DDR_PHY_OFFSET + 0x30) /* DRAM Configuration Register*/

#define rDDRP_DTPR0	(DDRC_IOBASE + DDR_PHY_OFFSET + 0x34) /* DRAM Timing Parameters Register 0 */
#define rDDRP_DTPR1	(DDRC_IOBASE + DDR_PHY_OFFSET + 0x38) /* DRAM Timing Parameters Register 1 */
#define rDDRP_DTPR2	(DDRC_IOBASE + DDR_PHY_OFFSET + 0x3c) /* DRAM Timing Parameters Register 2 */
#define rDDRP_MR0	(DDRC_IOBASE + DDR_PHY_OFFSET + 0x40) /* Mode Register 0 */
#define rDDRP_MR1	(DDRC_IOBASE + DDR_PHY_OFFSET + 0x44) /* Mode Register 1 */
#define rDDRP_MR2	(DDRC_IOBASE + DDR_PHY_OFFSET + 0x48) /* Mode Register 2 */
#define rDDRP_MR3	(DDRC_IOBASE + DDR_PHY_OFFSET + 0x4c) /* Mode Register 3 */

#define rDDRP_ODTCR	(DDRC_IOBASE + DDR_PHY_OFFSET + 0x50) /* ODT Configure Register */
#define rDDRP_DTAR	(DDRC_IOBASE + DDR_PHY_OFFSET + 0x54) /* Data Training Address Register */
#define rDDRP_DTDR0	(DDRC_IOBASE + DDR_PHY_OFFSET + 0x58) /* Data Training Data Register 0 */
#define rDDRP_DTDR1	(DDRC_IOBASE + DDR_PHY_OFFSET + 0x5c) /* Data Training Data Register 1 */

#define rDDRP_DCUAR	(DDRC_IOBASE + DDR_PHY_OFFSET + 0xc0) /* DCU Address Register */
#define rDDRP_DCUDR	(DDRC_IOBASE + DDR_PHY_OFFSET + 0xc4) /* DCU Data Register */
#define rDDRP_DCURR	(DDRC_IOBASE + DDR_PHY_OFFSET + 0xc8) /* DCU Run Register */
#define rDDRP_DCULR	(DDRC_IOBASE + DDR_PHY_OFFSET + 0xcc) /* DCU Loop Register */
#define rDDRP_DCUGCR	(DDRC_IOBASE + DDR_PHY_OFFSET + 0xd0) /* DCU Gerneral Configuration Register */
#define rDDRP_DCUTPR	(DDRC_IOBASE + DDR_PHY_OFFSET + 0xd4) /* DCU Timing Parameters Register */
#define rDDRP_DCUSR0	(DDRC_IOBASE + DDR_PHY_OFFSET + 0xd8) /* DCU Status Register 0 */
#define rDDRP_DCUSR1	(DDRC_IOBASE + DDR_PHY_OFFSET + 0xdc) /* DCU Status Register 1 */

#define rDDRP_DXGCR(n)		(DDRC_IOBASE + DDR_PHY_OFFSET + 0x1c0 + n * 0x40) /* DATX8 n General Configuration Register */
#define rDDRP_DXGSR0(n)		(DDRC_IOBASE + DDR_PHY_OFFSET + 0x1c4 + n * 0x40) /* DATX8 n General Status Register */
#define rDDRP_DXGSR1(n)		(DDRC_IOBASE + DDR_PHY_OFFSET + 0x1c8 + n * 0x40) /* DATX8 n General Status Register */
#define rDDRP_DXDQSTR(n)	(DDRC_IOBASE + DDR_PHY_OFFSET + 0x1d4 + n * 0x40) /* DATX8 n DQS Timing Register */


/* DDRC Configure Register (DDRC_CFG) */
#define DDRC_CFG_ROW1_BIT	27 /* Row Address Width */
#define DDRC_CFG_ROW1_MASK	(0x7 << DDRC_CFG_ROW1_BIT)
#define DDRC_CFG_COL1_BIT	24 /* Col Address Width */
#define DDRC_CFG_COL1_MASK	(0x7 << DDRC_CFG_COL1_BIT)
#define DDRC_CFG_BA1_BIT	23 /* Bank Address Width */
#define DDRC_CFG_BA1        (1 << DDRC_CFG_BA1_BIT)
#define DDRC_CFG_IMBA       (1 << 22)
#define DDRC_CFG_BL_8       (0x1 << 21)
#define DDRC_CFG_TYPE_BIT	17 /* Memory Device Type */
#define DDRC_CFG_TYPE_MDDR	(0x3 << DDRC_CFG_TYPE_BIT)
#define DDRC_CFG_TYPE_DDR2	(0x4 << DDRC_CFG_TYPE_BIT)
#define DDRC_CFG_TYPE_LPDDR2   (0x5 << DDRC_CFG_TYPE_BIT)
#define DDRC_CFG_TYPE_DDR3	   (0x6 << DDRC_CFG_TYPE_BIT)
#define DDRC_CFG_ODTEN         (0x1 << 16) /* DDR Control ODT Enable */
#define DDRC_CFG_MISPE				(0x1 << 15) /* Miss CS Protect */
#define DDRC_CFG_ROW_BIT			11
#define DDRC_CFG_ROW_MASK			(0x7 << DDRC_CFG_ROW_BIT)
#define DDRC_CFG_COL_BIT			8
#define DDRC_CFG_COL_MASK			(0x7 << DDRC_CFG_COL_BIT)
#define DDRC_CFG_CS1EN_BIT			7 /* DDR Chip-Select-1 Enable */
#define DDRC_CFG_CS0EN_BIT			6 /* DDR Chip-Select-0 Enable */
#define DDRC_CFG_CL_BIT				2 /* CAS Latency */
#define DDRC_CFG_BA0_BIT			1
#define DDRC_CFG_DW_BIT				0

/* DDRC Control Register (DDRC_CTRL) */
#define DDRC_CTRL_ACTPD			(0x1 << 15) /* Active Power-Down */
#define DDRC_CTRL_PDT_BIT			12	    /* Enter Power-Down Timer */
#define DDRC_CTRL_PDT_64			(0x4 << CTRL_PDT_BIT) /* after 64 tCK */
#define DDRC_CTRL_ACTSTP			(0x1 << 11) /* Acitive Clock-Stop */
#define DDRC_CTRL_DPD			(0x1 << 6)  /* Driver Mobile DDR device entering Deep-Power-Down mode */
#define DDRC_CTRL_SR				(0x1 << 5)  /* Driver DDR device entering Self-Refresh mode */
#define DDRC_CTRL_UNALIGN			(0x1 << 4)  /* Enable unaligned transfer */
#define DDRC_CTRL_ALH			(0x1 << 3)  /* Advanced Latency Hiding */
#define DDRC_CTRL_CKE			(0x1 << 1)  /* Control CKE pin */
#define DDRC_CTRL_RESET			(0x1 << 0)  /* Resetting DDR Controller */

/* DDRC Memory Map Config Register */
#define DDRC_MMAP_BASE_BIT		8 /* base address */
#define DDRC_MMAP_BASE_MASK		(0xff << DDRC_MMAP_BASE_BIT)
#define DDRC_MMAP_MASK_BIT		0 /* address mask */
#define DDRC_MMAP_MASK_MASK		(0xff << DDRC_MMAP_MASK_BIT)

/* DDRC Timing Config Register 1 */
#define DDRC_TIMING1_TRTP_BIT	24 /* READ to PRECHARGE command period. */
#define DDRC_TIMING1_TRTP_MASK	(0x3f << DDRC_TIMING1_TRTP_BIT)
#define DDRC_TIMING1_TWTR_BIT	16 /* WRITE to READ command delay. */
#define DDRC_TIMING1_TWTR_MASK	(0x3f << DDRC_TIMING1_TWTR_BIT)
  #define DDRC_TIMING1_TWTR_1		(0 << DDRC_TIMING1_TWTR_BIT)
  #define DDRC_TIMING1_TWTR_2		(1 << DDRC_TIMING1_TWTR_BIT)
  #define DDRC_TIMING1_TWTR_3		(2 << DDRC_TIMING1_TWTR_BIT)
  #define DDRC_TIMING1_TWTR_4		(3 << DDRC_TIMING1_TWTR_BIT)
#define DDRC_TIMING1_TWR_BIT 	8 /* WRITE Recovery Time defined by register MR of DDR2 DDR3 memory */
#define DDRC_TIMING1_TWR_MASK	(0x3f << DDRC_TIMING1_TWR_BIT)
  #define DDRC_TIMING1_TWR_1		(0 << DDRC_TIMING1_TWR_BIT)
  #define DDRC_TIMING1_TWR_2		(1 << DDRC_TIMING1_TWR_BIT)
  #define DDRC_TIMING1_TWR_3		(2 << DDRC_TIMING1_TWR_BIT)
  #define DDRC_TIMING1_TWR_4		(3 << DDRC_TIMING1_TWR_BIT)
  #define DDRC_TIMING1_TWR_5		(4 << DDRC_TIMING1_TWR_BIT)
  #define DDRC_TIMING1_TWR_6		(5 << DDRC_TIMING1_TWR_BIT)
#define DDRC_TIMING1_TWL_BIT 	0 /* Write latency = RL - 1 */
#define DDRC_TIMING1_TWL_MASK	(0x3f << DDRC_TIMING1_TWL_BIT)

/* DDRC Timing Config Register 2 */
#define DDRC_TIMING2_TCCD_BIT 	24 /* CAS# to CAS# command delay */
#define DDRC_TIMING2_TCCD_MASK 	(0x3f << DDRC_TIMING2_TCCD_BIT)
#define DDRC_TIMING2_TRAS_BIT 	16 /* ACTIVE to PRECHARGE command period (2 * tRAS + 1) */
#define DDRC_TIMING2_TRAS_MASK 	(0x3f << DDRC_TIMING2_TRAS_BIT)
#define DDRC_TIMING2_TRCD_BIT	8  /* ACTIVE to READ or WRITE command period. */
#define DDRC_TIMING2_TRCD_MASK	(0x3f << DDRC_TIMING2_TRCD_BIT)
#define DDRC_TIMING2_TRL_BIT	0  /* Read latency = AL + CL*/
#define DDRC_TIMING2_TRL_MASK	(0x3f << DDRC_TIMING2_TRL_BIT)

/* DDRC Timing Config Register 3 */
#define DDRC_TIMING3_ONUM   27
#define DDRC_TIMING3_TCKSRE_BIT		24 /* Valid clock after enter self-refresh. */
#define DDRC_TIMING3_TCKSRE_MASK 	(0x3f << DDRC_TIMING3_TCKSRE_BIT)
#define DDRC_TIMING3_TRP_BIT	16 /* PRECHARGE command period. */
#define DDRC_TIMING3_TRP_MASK 	(0x3f << DDRC_TIMING3_TRP_BIT)
#define DDRC_TIMING3_TRRD_BIT	8 /* ACTIVE bank A to ACTIVE bank B command period. */
#define DDRC_TIMING3_TRRD_MASK	(0x3f << DDRC_TIMING3_TRRD_BIT)
  #define DDRC_TIMING3_TRRD_DISABLE	(0 << DDRC_TIMING3_TRRD_BIT)
  #define DDRC_TIMING3_TRRD_2		(1 << DDRC_TIMING3_TRRD_BIT)
  #define DDRC_TIMING3_TRRD_3		(2 << DDRC_TIMING3_TRRD_BIT)
  #define DDRC_TIMING3_TRRD_4		(3 << DDRC_TIMING3_TRRD_BIT)
#define DDRC_TIMING3_TRC_BIT 	0 /* ACTIVE to ACTIVE command period. */
#define DDRC_TIMING3_TRC_MASK 	(0x3f << DDRC_TIMING3_TRC_BIT)

/* DDRC Timing Config Register 4 */
#define DDRC_TIMING4_TRFC_BIT         24 /* AUTO-REFRESH command period. */
#define DDRC_TIMING4_TRFC_MASK        (0x3f << DDRC_TIMING4_TRFC_BIT)
#define DDRC_TIMING4_TEXTRW_BIT	      21 /* ??? */
#define DDRC_TIMING4_TEXTRW_MASK      (0x7 << DDRC_TIMING4_TEXTRW_BIT)
#define DDRC_TIMING4_TRWCOV_BIT	      19 /* ??? */
#define DDRC_TIMING4_TRWCOV_MASK      (0x3 << DDRC_TIMING4_TRWCOV_BIT)
#define DDRC_TIMING4_TCKE_BIT	      16 /* ??? */
#define DDRC_TIMING4_TCKE_MASK        (0x7 << DDRC_TIMING4_TCKE_BIT)
#define DDRC_TIMING4_TMINSR_BIT       8  /* Minimum Self-Refresh / Deep-Power-Down time */
#define DDRC_TIMING4_TMINSR_MASK      (0xf << DDRC_TIMING4_TMINSR_BIT)
#define DDRC_TIMING4_TXP_BIT          4  /* EXIT-POWER-DOWN to next valid command period. */
#define DDRC_TIMING4_TXP_MASK         (0x7 << DDRC_TIMING4_TXP_BIT)
#define DDRC_TIMING4_TMRD_BIT         0  /* Load-Mode-Register to next valid command period. */
#define DDRC_TIMING4_TMRD_MASK        (0x3 << DDRC_TIMING4_TMRD_BIT)

/* DDRC Timing Config Register 5 */
#define DDRC_TIMING5_TCTLUPD_BIT	24 /* ??? */
#define DDRC_TIMING4_TCTLUPD_MASK	(0x3f << DDRC_TIMING5_TCTLUDP_BIT)
#define DDRC_TIMING5_TRTW_BIT		16 /* ??? */
#define DDRC_TIMING5_TRTW_MASK		(0x3f << DDRC_TIMING5_TRTW_BIT)
#define DDRC_TIMING5_TRDLAT_BIT		8 /* RL - 2 */
#define DDRC_TIMING5_TRDLAT_MASK	(0x3f << DDRC_TIMING5_TRDLAT_BIT)
#define DDRC_TIMING5_TWDLAT_BIT		0 /* WL - 1 */
#define DDRC_TIMING5_TWDLAT_MASK	(0x3f << DDRC_TIMING5_TWDLAT_BIT)

/* DDRC Timing Config Register 6 */
#define DDRC_TIMING6_TXSRD_BIT		24 /* exit power-down to READ delay */
#define DDRC_TIMING6_TXSRD_MASK		(0x3f << DDRC_TIMING6_TXSRD_BIT)
#define DDRC_TIMING6_TFAW_BIT		16 /* 4-active command window */
#define DDRC_TIMING6_TFAW_MASK		(0x3f << DDRC_TIMING6_TFAW_BIT)
#define DDRC_TIMING6_TCFGW_BIT		8 /* Write PHY configure registers to other commands delay */
#define DDRC_TIMING6_TCFGW_MASK		(0x3f << DDRC_TIMING6_TCFGW_BIT)
#define DDRC_TIMING6_TCFGR_BIT		0 /* Ready PHY configure registers to other commands delay */
#define DDRC_TIMING6_TCFGR_MASK		(0x3f << DDRC_TIMING6_TCFGR_BIT)

/* DDR3 Mode Register Set*/
#define DDR3_MR0_BL_BIT		0
#define DDR3_MR0_BL_MASK	(3 << DDR3_MR0_BL_BIT)
  #define DDR3_MR0_BL_8		(0 << DDR3_MR0_BL_BIT)
  #define DDR3_MR0_BL_fly	(1 << DDR3_MR0_BL_BIT)
  #define DDR3_MR0_BL_4		(2 << DDR3_MR0_BL_BIT)
#define DDR3_MR0_BT_BIT		3
#define DDR3_MR0_BT_MASK	(1 << DDR3_MR0_BT_BIT)
  #define DDR3_MR0_BT_SEQ 	(0 << DDR3_MR0_BT_BIT)
  #define DDR3_MR0_BT_INTER 	(1 << DDR3_MR0_BT_BIT)
#define DDR3_MR0_WR_BIT		9

#define DDR3_MR1_DLL_DISABLE	1
#define DDR3_MR1_DIC_6 		(0 << 5 | 0 << 1)
#define DDR3_MR1_DIC_7 		(0 << 5 | 1 << 1)
#define DDR3_MR1_RTT_DIS	(0 << 9 | 0 << 6 | 0 << 2)
#define DDR3_MR1_RTT_4 		(0 << 9 | 0 << 6 | 1 << 2)
#define DDR3_MR1_RTT_2 		(0 << 9 | 1 << 6 | 0 << 2)
#define DDR3_MR1_RTT_6 		(0 << 9 | 1 << 6 | 1 << 2)
#define DDR3_MR1_RTT_12 	(1 << 9 | 0 << 6 | 0 << 2)
#define DDR3_MR1_RTT_8 		(1 << 9 | 0 << 6 | 1 << 2)

#define DDR3_MR2_CWL_BIT	3

/* DDRP PHY General Configurate Register */
#define DDRP_PGCR_ITMDMD	(1 << 0)
#define DDRP_PGCR_DQSCFG	(1 << 1)
#define DDRP_PGCR_DFTCMP	(1 << 2)
#define DDRP_PGCR_DFTLMT_BIT	3
#define DDRP_PGCR_DTOSEL_BIT	5
#define DDRP_PGCR_CKEN_BIT      9
#define DDRP_PGCR_CKDV_BIT	    12
#define DDRP_PGCR_CKINV		    (1 << 14)
#define DDRP_PGCR_RANKEN_BIT	18
#define DDRP_PGCR_ZCKSEL_32	    (2 << 22)
#define DDRP_PGCR_PDDISDX	    (1 << 24)

/* DDRP PHY General Status Register */
#define DDRP_PGSR_IDONE		(1 << 0)
#define DDRP_PGSR_DLDONE	(1 << 1)
#define DDRP_PGSR_ZCDONE	(1 << 2)
#define DDRP_PGSR_DIDONE	(1 << 3)
#define DDRP_PGSR_DTDONE	(1 << 4)
#define DDRP_PGSR_DTERR 	(1 << 5)
#define DDRP_PGSR_DTIERR 	(1 << 6)
#define DDRP_PGSR_DFTEERR 	(1 << 7)

/* DDRP PHY Initialization Register */
#define DDRP_PIR_INIT		(1 << 0)
#define DDRP_PIR_DLLSRST	(1 << 1)
#define DDRP_PIR_DLLLOCK	(1 << 2)
#define DDRP_PIR_ZCAL   	(1 << 3)
#define DDRP_PIR_ITMSRST   	(1 << 4)
#define DDRP_PIR_DRAMRST   	(1 << 5)
#define DDRP_PIR_DRAMINT   	(1 << 6)
#define DDRP_PIR_QSTRN   	(1 << 7)
#define DDRP_PIR_EYETRN   	(1 << 8)
#define DDRP_PIR_DLLBYP   	(1 << 17)

#define DDRC_CFG_CS1EN		(1 << 7) /* 0 DDR Pin CS1 un-used
					    1 There're DDR memory connected to CS1 */
#define DDRC_CFG_CS0EN		(1 << 6) /* 0 DDR Pin CS0 un-used
					    1 There're DDR memory connected to CS0 */
#define DDRC_CFG_CL_BIT		2 /* CAS Latency */
#define DDRC_CFG_CL_MASK	(0xf << DDRC_CFG_CL_BIT)
#define DDRC_CFG_CL_3		(0 << DDRC_CFG_CL_BIT) /* CL = 3 tCK */
#define DDRC_CFG_CL_4		(1 << DDRC_CFG_CL_BIT) /* CL = 4 tCK */
#define DDRC_CFG_CL_5		(2 << DDRC_CFG_CL_BIT) /* CL = 5 tCK */
#define DDRC_CFG_CL_6		(3 << DDRC_CFG_CL_BIT) /* CL = 6 tCK */

#define DDRC_CFG_BA		(1 << 1) /* 0 4 bank device, Pin ba[1:0] valid, ba[2] un-used
					    1 8 bank device, Pin ba[2:0] valid*/
#define DDRC_CFG_DW		(1 << 0) /*0 External memory data width is 16-bit
					   1 External memory data width is 32-bit */

/* DDRC  Auto-Refresh Counter */
#define DDRC_REFCNT_CON_BIT     16 /* Constant value used to compare with CNT value. */
#define DDRC_REFCNT_REF_EN      (1 << 0) /* Enable Refresh Counter */

/* Register access macros */
#define ddr_readl(base, reg)		    \
	readl(base + DDRC_##reg)
#define ddr_writel(base, reg, value)    \
	writel((value), base + DDRC_##reg)

#endif /* __JZ4780_DDR_H__ */


