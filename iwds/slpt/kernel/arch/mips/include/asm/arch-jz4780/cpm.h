/*
 * JZSOC CPM register definition.
 *
 * CPM (Clock reset and Power control Management)
 * from kernel/arch/mips/xburst/soc-4780/include/soc/cpm.h
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co., Ltd.
 * Written by Jiang Tao <tjiang@ingenic.cn>
 * 				Kage Shen <kkshen@ingenic.cn>
 */

#ifndef __JZ4780_CPM_H__
#define __JZ4780_CPM_H__

/* CPM Controller driver registers */
struct jz4780_cpm_regs {
	volatile u32 cpccr;         /* 00 */
	volatile u32 lcr;           /* 04 */
	volatile u32 rsr;           /* 08 */
	volatile u32 cppcr;         /* 0c */
	volatile u32 cpapcr;        /* 10 */
	volatile u32 cpmpcr;        /* 14 */
	volatile u32 cpepcr;        /* 18 */
	volatile u32 cpvpcr;        /* 1c */
	volatile u32 clkgr0;        /* 20 */
	volatile u32 opcr;          /* 24 */
	volatile u32 clkgr1;        /* 28 */
	volatile u32 ddrcdr;        /* 2c */
	volatile u32 vpucdr;		/* 30 */
	volatile u32 cppsr;			/* 34 */
	volatile u32 cpsppr;		/* 38 */
	volatile u32 usbpcr;		/* 3c */
	volatile u32 usbrdt;		/* 40 */
	volatile u32 usbvbfil;		/* 44 */
	volatile u32 usbpcr1;		/* 48 */
	volatile u32 padding3[(0x54-0x4c) / sizeof(u32)];
	volatile u32 lcd0cdr;		/* 54 */
	volatile u32 padding4[(0x60-0x58) / sizeof(u32)];
	volatile u32 i2scdr;		/* 60 */
	volatile u32 lcd1cdr;		/* 64 */
	volatile u32 msc0cdr;		/* 68 */
	volatile u32 uhccdr;		/* 6c */
	volatile u32 padding5;
	volatile u32 ssicdr;		/* 74 */
	volatile u32 padding6;
	volatile u32 cimcdr;		/* 7c */
	volatile u32 padding7;
	volatile u32 pcmcdr;		/* 84 */
	volatile u32 gpucdr;		/* 88 */
	volatile u32 hdmicdr;		/* 8c */
	volatile u32 pswc0st;		/* 90 */
	volatile u32 pswc1st;		/* 94 */
	volatile u32 pswc2st;		/* 98 */
	volatile u32 pswc3st;		/* 9c */
	volatile u32 padding8;
	volatile u32 msc1cdr;		/* a4 */
	volatile u32 msc2cdr;		/* a8 */
	volatile u32 bchcdr;		/* ac */
	volatile u32 intr;			/* b0 */
	volatile u32 intre;			/* b4 */
	volatile u32 spcr0;			/* b8 */
	volatile u32 spcr1;			/* bc */
	volatile u32 padding9[(0xd4-0xc0) / sizeof(u32)];
	volatile u32 cpcsr;			/* d4 */
	volatile u32 erng;			/* d8 */
	volatile u32 rng;			/* dc */

};

/* Clock Gate Register0 */
#define CPM_CLKGR0_DDR1		(1 << 31)
#define CPM_CLKGR0_DDR0 	(1 << 30)
#define CPM_CLKGR0_IPU    	(1 << 29)
#define CPM_CLKGR0_LCD		(1 << 28)
#define CPM_CLKGR0_TVE  	(1 << 27)
#define CPM_CLKGR0_CIM    	(1 << 26)
#define CPM_CLKGR0_I2C2    	(1 << 25)
#define CPM_CLKGR0_UHC    	(1 << 24)
#define CPM_CLKGR0_MAC    	(1 << 23)
#define CPM_CLKGR0_GPS    	(1 << 22)
#define CPM_CLKGR0_PDMAC    	(1 << 21)
#define CPM_CLKGR0_SSI2    	(1 << 20)
#define CPM_CLKGR0_SSI1    	(1 << 19)
#define CPM_CLKGR0_UART3    	(1 << 18)
#define CPM_CLKGR0_UART2    	(1 << 17)
#define CPM_CLKGR0_UART1    	(1 << 16)
#define CPM_CLKGR0_UART0	(1 << 15)
#define CPM_CLKGR0_SADC		(1 << 14)
#define CPM_CLKGR0_KBC		(1 << 13)
#define CPM_CLKGR0_MSC2		(1 << 12)
#define CPM_CLKGR0_MSC1		(1 << 11)
#define CPM_CLKGR0_OWI		(1 << 10)
#define CPM_CLKGR0_TSSI		(1 << 9)
#define CPM_CLKGR0_AIC		(1 << 8)
#define CPM_CLKGR0_SCC		(1 << 7)
#define CPM_CLKGR0_I2C1		(1 << 6)
#define CPM_CLKGR0_I2C0		(1 << 5)
#define CPM_CLKGR0_SSI0		(1 << 4)
#define CPM_CLKGR0_MSC0		(1 << 3)
#define CPM_CLKGR0_OTG		(1 << 2)
#define CPM_CLKGR0_BCH		(1 << 1)
#define CPM_CLKGR0_NEMC		(1 << 0)

/* Clock Gate Register1 */
#define CPM_CLKGR1_P1		(1 << 15)
#define CPM_CLKGR1_X2D		(1 << 14)
#define CPM_CLKGR1_DES		(1 << 13)
#define CPM_CLKGR1_I2C4		(1 << 12)
#define CPM_CLKGR1_AHB		(1 << 11)
#define CPM_CLKGR1_UART4	(1 << 10)
#define CPM_CLKGR1_HDMI		(1 << 9)
#define CPM_CLKGR1_OTG1		(1 << 8)
#define CPM_CLKGR1_GPVLC	(1 << 7)
#define CPM_CLKGR1_AIC1 	(1 << 6)
#define CPM_CLKGR1_COMPRES	(1 << 5)
#define CPM_CLKGR1_GPU		(1 << 4)
#define CPM_CLKGR1_PCM		(1 << 3)
#define CPM_CLKGR1_VPU		(1 << 2)
#define CPM_CLKGR1_TSSI1	(1 << 1)
#define CPM_CLKGR1_I2C3		(1 << 0)


#define LCR_LPM_MASK		(0x3)
#define LCR_LPM_SLEEP		(0x1)

#define CPM_LCR_PD_SCPU		(0x1<<31)
#define CPM_LCR_PD_VPU		(0x1<<30)
#define CPM_LCR_PD_GPU		(0x1<<29)
#define CPM_LCR_PD_GPS		(0x1<<28)
#define CPM_LCR_PD_MASK		(0x7<<28)
#define CPM_LCR_SCPUS 		(0x1<<27)
#define CPM_LCR_VPUS		(0x1<<26)
#define CPM_LCR_GPUS		(0x1<<25)
#define CPM_LCR_GPSS 		(0x1<<25)
#define CPM_LCR_STATUS_MASK (0xf<<24)
#define CPM_LCR_GPU_IDLE 	(0x1<<24)

#define OPCR_ERCS		(0x1<<2)
#define OPCR_PD			(0x1<<3)
#define OPCR_IDLE		(0x1<<31)

#define CLKGR1_VPU   	(0x1<<2)

/* Clock control register */
#define CPCCR_SEL_SRC_BIT	    30
#define CPCCR_SEL_SRC_MASK	    (0x3 << CPCCR_SEL_SRC_BIT)
#define CPCCR_SRC_SEL_STOP		0
#define CPCCR_SRC_SEL_APLL  	1
#define CPCCR_SRC_SEL_EXCLK		2
#define CPCCR_SRC_SEL_RTCLK		3
#define CPCCR_SEL_CPLL_BIT	    28
#define CPCCR_SEL_H0PLL_BIT	    26
#define CPCCR_SEL_H2PLL_BIT	    24
#define CPCCR_PLL_SEL_STOP		0
#define CPCCR_PLL_SEL_SRC		1
#define CPCCR_PLL_SEL_MPLL		2
#define CPCCR_PLL_SEL_EPLL		3
#define CPCCR_CE_CPU		    (0x1 << 22)
#define CPCCR_CE_AHB0		    (0x1 << 21)
#define CPCCR_CE_AHB2		    (0x1 << 20)
#define CPCCR_PDIV_BIT		    16
#define CPCCR_PDIV_MASK		    (0xf << CPCCR_PDIV_BIT)
#define CPCCR_H2DIV_BIT		    12
#define CPCCR_H2DIV_MASK	    (0xf << CPCCR_H2DIV_BIT)
#define CPCCR_H0DIV_BIT		    8
#define CPCCR_H0DIV_MASK	    (0xf << CPCCR_H0DIV_BIT)
#define CPCCR_L2DIV_BIT		    4
#define CPCCR_L2DIV_MASK	    (0xf << CPCCR_L2DIV_BIT)
#define CPCCR_CDIV_BIT		    0
#define CPCCR_CDIV_MASK		    (0xf << CPCCR_CDIV_BIT)

/* Clock Status register */
#define CPCSR_H2DIV_BUSY	    (1 << 2)
#define CPCSR_H0DIV_BUSY	    (1 << 1)
#define CPCSR_CDIV_BUSY		    (1 << 0)

/* XPLL control register */
#define CPXPCR_XPLLM_BIT	19
#define CPXPCR_XPLLM_MASK	(0x1fff << CPXPCR_XPLLM_BIT)
#define CPXPCR_XPLLN_BIT	13
#define CPXPCR_XPLLN_MASK	(0x3f << CPXPCR_XPLLN_BIT)
#define CPXPCR_XPLLOD_BIT	9
#define CPXPCR_XPLLOD_MASK	(0xf << CPXPCR_XPLLOD_BIT)
#define CPXPCR_XLOCK		(1 << 6)
#define CPXPCR_XPLL_ON		(1 << 4)
#define CPXPCR_XF_MODE		(1 << 3)
#define CPXPCR_XPLLBP		(1 << 1)
#define CPXPCR_XPLLEN		(1 << 0)

/* DDR memory clock divider register */
#define CPM_DDRCDR_STOP     (0x0 << 30)
#define CPM_DDRCDR_APLL     (1 << 30)
#define CPM_DDRCDR_MPLL     (2 << 30)
#define CPM_DDRCDR_CE       (1 << 29)
#define CPM_DDRCDR_BUSY     (1 << 28)

#define CPM_IOBASE 			0xb0000000

struct clk;

struct clk_ops {
	unsigned long (*get_rate)(struct clk *clk);
	unsigned long (*round_rate)(struct clk *clk, unsigned long rate);
	/* Set the clock to the requested clock rate */
	int (*set_rate)(struct clk *clk, unsigned long rate);
	int (*enable)(struct clk *clk);
	int (*disable)(struct clk *clk);
	int (*is_enabled)(struct clk *clk);

	int (*set_power)(struct clk *clk, unsigned int on);
	int (*is_power_on)(struct clk *clk);

	/* Set the clock's parent to another clock source */
	int (*set_parent)(struct clk *clk, struct clk *parent);
	/* Retrieve the clock's parent clock source */
	struct clk *(*get_parent)(struct clk *clk);
};

struct reg_description {
	unsigned int reg_offset;
	unsigned int ce;             /* CE offset */
	unsigned int stop;           /* STOP offset */
	unsigned int div_mul;        /* clock multiplier */
	unsigned int div_width;      /* div width */
	unsigned int ext;            /* ext offset */
	unsigned int *ext_list;      /* parent type list */
	unsigned int busy;           /* busy offset */
};

struct clk {
	char *name;
	unsigned int pow_cgr;
	unsigned int parent;
	const struct clk_ops *ops;
	const struct reg_description *reg_des;
};

extern struct clk *clk_get(const char *dev_id);
extern int clk_enable(struct clk *clk);
extern int clk_is_enabled(struct clk *clk);
extern int clk_disable(struct clk *clk);
extern int clk_set_rate(struct clk *clk, unsigned long rate);
extern unsigned long clk_get_rate(struct clk *clk);

extern int clk_set_parent(struct clk *clk, struct clk *parent);
extern struct clk *clk_get_parent(struct clk *clk);
#endif /* __JZ4780_CPM_H__ */

