/*
 * JZSOC CPM register definition.
 *
 * CPM (Clock reset and Power control Management)
 * from kernel/arch/mips/xburst/soc-m200/include/soc/cpm.h
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co., Ltd.
 * Written by Jiang Tao <tjiang@ingenic.cn>
 * 			  Kage Shen <kkshen@ingenic.cn>
 * 			  ZhangYanMing <ymzhang@ingenic.cn>
 */

#ifndef __M200_CPM_H__
#define __M200_CPM_H__

/* CPM Controller driver registers */
struct jz4775_cpm_regs {
	volatile u32 cpccr;			//00
	volatile u32 lcr;			//04
	volatile u32 rsr;			//08
	volatile u32 cppcr;			//0c
	volatile u32 cpapcr;		//10
	volatile u32 cpmpcr;		//14
	volatile u32 padding0[(0x20-0x18) / sizeof(u32)];
	volatile u32 clkgr0;		//20
	volatile u32 opcr;			//24
	volatile u32 padding1;		//28
	volatile u32 ddrcdr;		//2c
	volatile u32 vpucdr;		//30
	volatile u32 cpppr;			//34
	volatile u32 cpsppr;		//38
	volatile u32 usbpcr;		//3c
	volatile u32 usbrdt;		//40
	volatile u32 usbvbfil;		//44
	volatile u32 usbpcr1;		//48
	volatile u32 padding2;		//4c
	volatile u32 usbcdr;		//50
	volatile u32 padding3[(0x60-0x54) / sizeof(u32)];
	volatile u32 i2scdr;		//60
	volatile u32 lcdcdr;		//64
	volatile u32 msc0cdr;		//68
	volatile u32 uhccdr;		//6c
	volatile u32 padding4;		//70
	volatile u32 ssicdr;		//74
	volatile u32 padding5;		//78
	volatile u32 cimcdr;		//7c
	volatile u32 cim1cdr;		//80
	volatile u32 pcmcdr;		//84
	volatile u32 padding6[(0x90-0x88) / sizeof(u32)];
	volatile u32 pswc0st;		//90
	volatile u32 pswc1st;		//94
	volatile u32 pswc2st;		//98
	volatile u32 pswc3st;		//9c
	volatile u32 i2s1cdr;		//a0
	volatile u32 msc1cdr;		//a4
	volatile u32 msc2cdr;		//a8
	volatile u32 bchcdr;		//ac
	volatile u32 intr;			//b0
	volatile u32 intre;			//b4
	volatile u32 spcr0;			//b8
	volatile u32 spcr1;			//bc
	volatile u32 padding7;		//c0
	volatile u32 srbc;			//c4
	volatile u32 slbc;			//c8
	volatile u32 slpc;			//cc
	volatile u32 padding8;		//d0
	volatile u32 cpcsr;			//d4
	volatile u32 erng;			//d8
	volatile u32 rng;			//dc

};

/* Clock Gate Register */
#define CPM_CLKGR0_DDR		(1 << 31)
#define CPM_CLKGR0_EPDE  	(1 << 27)
#define CPM_CLKGR0_EPDC    	(1 << 26)
#define CPM_CLKGR0_LCD    	(1 << 25)
#define CPM_CLKGR0_CIM1    	(1 << 24)
#define CPM_CLKGR0_CIM0    	(1 << 23)
#define CPM_CLKGR0_UHC    	(1 << 22)
#define CPM_CLKGR0_GMAC    	(1 << 21)
#define CPM_CLKGR0_PDMA    	(1 << 20)
#define CPM_CLKGR0_VPU    	(1 << 19)
#define CPM_CLKGR0_UART3	(1 << 18)
#define CPM_CLKGR0_UART2	(1 << 17)
#define CPM_CLKGR0_UART1	(1 << 16)
#define CPM_CLKGR0_UART0	(1 << 15)
#define CPM_CLKGR0_SADC		(1 << 14)
#define CPM_CLKGR0_PCM		(1 << 13)
#define CPM_CLKGR0_MSC2		(1 << 12)
#define CPM_CLKGR0_MSC1		(1 << 11)
#define CPM_CLKGR0_AHB		(1 << 10)
#define CPM_CLKGR0_X2D		(1 << 9)
#define CPM_CLKGR0_AIC		(1 << 8)
#define CPM_CLKGR0_I2C2		(1 << 7)
#define CPM_CLKGR0_I2C1		(1 << 6)
#define CPM_CLKGR0_I2C0		(1 << 5)
#define CPM_CLKGR0_SSI0		(1 << 4)
#define CPM_CLKGR0_MSC0		(1 << 3)
#define CPM_CLKGR0_OTG		(1 << 2)
#define CPM_CLKGR0_BCH		(1 << 1)
#define CPM_CLKGR0_NEMC		(1 << 0)



/* Low Power Control Register */
#define LCR_LPM_MASK		(0x3)
#define LCR_LPM_SLEEP		(0x1)
#define CPM_LCR_PD_X2D		(0x1<<31)
#define CPM_LCR_PD_VPU		(0x1<<30)
#define CPM_LCR_PD_MASK		(0x3<<30)
#define CPM_LCR_X2DS 		(0x1<<27)
#define CPM_LCR_VPUS		(0x1<<26)
#define CPM_LCR_STATUS_MASK (0x3<<26)

/* Oscillator and Power Control Register */
#define OPCR_ERCS		(0x1<<2)
#define OPCR_PD			(0x1<<3)
#define OPCR_IDLE		(0x1<<31)

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

/* APLL control register */
#define CPXPCR_BS_BIT		19
#define CPXPCR_PLLM_BIT		24
#define CPXPCR_PLLM_MASK	(0x7f << CPXPCR_PLLM_BIT)
#define CPXPCR_PLLN_BIT		18
#define CPXPCR_PLLN_MASK	(0x1f << CPXPCR_PLLN_BIT)
#define CPXPCR_PLLOD_BIT	16
#define CPXPCR_PLLOD_MASK	(2 << CPXPCR_PLLOD_BIT)

#define CPAPCR_LOCK			(1 << 15)
#define CPAPCR_PLLON		(1 << 10)
#define CPAPCR_PLLBP		(1 << 9)
#define CPAPCR_PLLEN		(1 << 8)
#define CPAPCR_PLLST_BIT	0

/* MPLL Control Register */
#define CPMPCR_LOCK			(1 << 1)
#define CPMPCR_PLLON		(1 << 0)
#define CPMPCR_PLLBP		(1 << 6)
#define CPMPCR_PLLEN		(1 << 7)


/* DDR memory clock divider register */
#define CPM_DDRCDR_STOP     (0x0 << 30)
#define CPM_DDRCDR_APLL     (1 << 30)
#define CPM_DDRCDR_MPLL     (2 << 30)
#define CPM_DDRCDR_CE       (1 << 29)
#define CPM_DDRCDR_BUSY     (1 << 28)

#define CPM_IOBASE 			0xb0000000

struct clk {
	char *name;
	unsigned int pow_cgr;
	unsigned int parent;
	const struct clk_ops *ops;
	const struct reg_description *reg_des;
};

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

extern struct clk *clk_get(const char *dev_id);
extern int clk_enable(struct clk *clk);
extern int clk_is_enabled(struct clk *clk);
extern int clk_disable(struct clk *clk);
extern int clk_set_rate(struct clk *clk, unsigned long rate);
extern unsigned long clk_get_rate(struct clk *clk);

extern int clk_set_parent(struct clk *clk, struct clk *parent);
extern struct clk *clk_get_parent(struct clk *clk);
#endif /* __M200_CPM_H__ */

