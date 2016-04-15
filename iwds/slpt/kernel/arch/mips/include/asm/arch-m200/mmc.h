/*
 * Ingenic MMC/SD Controller driver head file
 * from kernel/drivers/mmc/host/m200_mmc.h
 *
 * Copyright (C) 2012 Ingenic Semiconductor Co., Ltd.
 * Written by Large Dipper <ykli@ingenic.cn>.
 * 			Kage Shen <kkshen@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __M200_MMC_H__
#define __M200_MMC_H__

/* MMC/SD Controller driver registers */
struct hsmmc {
	volatile u32 ctrl;		/* 0x00 */
	volatile u32 stat;		/* 0x04 */
	volatile u32 clkrt;		/* 0x08 */
	volatile u32 cmdat;		/* 0x0c */
	volatile u32 resto;		/* 0x10 */
	volatile u32 rdto;		/* 0x14 */
	volatile u32 blklen;	/* 0x18 */
	volatile u32 nob;		/* 0x1c */
	volatile u32 snob;		/* 0x20 */
	volatile u32 imask;		/* 0x24 */
	volatile u32 ireg;		/* 0x28 */
	volatile u32 cmd;		/* 0x2c */
	volatile u32 arg;		/* 0x30 */
	volatile u32 res;		/* 0x34 */
	volatile u32 rxfifo;	/* 0x38 */
	volatile u32 txfifo;	/* 0x3c */
	volatile u32 lpm;		/* 0x40 */
	volatile u32 dmac;		/* 0x44 */
	volatile u32 dmanda;	/* 0x48 */
	volatile u32 dmada;		/* 0x4c */
	volatile u32 dmalen;	/* 0x50 */
	volatile u32 dmacmd;	/* 0x54 */
	volatile u32 ctrl2;		/* 0x58 */
	volatile u32 rtcnt;		/* 0x5c */
	volatile u32 padding1[(0xfc-0x60) >> sizeof(u32)];
	volatile u32 debug;		/* 0xfc */
};

struct m200_mmc_host {
	struct mmc *mmc;

	struct hsmmc *regs;
	struct clk *clk;

};

#endif /* __M200_MMC_H__ */
