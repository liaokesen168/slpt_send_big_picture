/*
 * Copyright (C) 2013 Ingenic Semiconductor Co., Ltd.
 * Authors: Kage Shen <kkshen@ingenic.cn>
 *          ZhangYanMing <ymzhang@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 3 of the License, or (at your option) any later version.
 */

#include <config.h>
#include <common.h>
#include <malloc.h>
#include <command.h>

#include <linux/list.h>
#include <linux/string.h>

#include <linux/err.h>
#include <asm/io.h>

#include <asm/arch/cpm.h>
#include <asm/arch/board_special.h>

#if ! defined offsetof
#define offsetof(s,m)   ((unsigned int)&(((s *)0)->m))
#endif // offsetof

struct jz4775_cpm_regs *cpm_reg = (struct jz4775_cpm_regs *)CPM_IOBASE;

enum {
	PAND = 0,

	/* clock source */
	CLK_EXT0,
	CLK_EXT1,
	CLK_APLL,
	CLK_MPLL,

	CLK_SCLKA,
	CLK_CCLK,
	CLK_L2CLK,
	CLK_H0CLK,
	CLK_H2CLK,
	CLK_PCLK,

	CLK_NEMC,
	CLK_BCH,
	CLK_OTG,
	CLK_MSC0,
	CLK_SSI0,
	CLK_I2C0,
	CLK_I2C1,
	CLK_I2C2,
	CLK_AIC,
	CLK_X2D,
	CLK_AHB_MON,
	CLK_MSC1,
	CLK_MSC2,
	CLK_PCM,
	CLK_SADC,
	CLK_UART0,
	CLK_UART1,
	CLK_UART2,
	CLK_UART3,
	CLK_VPU,
	CLK_PDMA,
	CLK_GMAC,
	CLK_UHC,
	CLK_CIM0,
	CLK_CIM1,
	CLK_LCD,
	CLK_EPDC,
	CLK_EPDE,
	CLK_DDR,
	END_ALL
};

static int com_set_parent(struct clk *c, struct clk *p);
static struct clk *com_get_parent(struct clk *c);

static int ext_is_enabled(struct clk *c);
static unsigned long ext_get_rate(struct clk *c);

static struct clk_ops ext_ops = {
		.is_enabled = ext_is_enabled,
		.is_power_on = ext_is_enabled,
		.get_rate = ext_get_rate
};

static int pll_is_enabled(struct clk *c);
static unsigned long pll_get_rate(struct clk *c);

static struct clk_ops pll_ops = {
		.is_power_on = pll_is_enabled,
		.is_enabled = pll_is_enabled,
		.get_rate = pll_get_rate,
		.get_parent = com_get_parent,
};

static unsigned long cr_get_rate(struct clk *clk);
static int cr_is_enabled(struct clk *c);

#define SCLKA_APLL (1)
#define SCLKA_EXT0 (2)
#define SCLKA_EXT1 (3)

#define CPLL_SCLKA (1)
#define CPLL_MPLL  (2)

#define H2PLL_SCLKA (1)
#define H2PLL_MPLL  (2)

static struct clk_ops cr_ops = {
		.is_power_on = cr_is_enabled,
		.is_enabled = cr_is_enabled,
		.get_rate = cr_get_rate,
		.get_parent = com_get_parent,
};

static unsigned long get_pll_rate(u32 pll_mode);

static int com_set_power(struct clk *c, u32);
static int com_is_power_on(struct clk *c);
static int com_is_enabled(struct clk *c);

static int com_enable(struct clk *c);
static int cgu_enable(struct clk *c);
static int com_disable(struct clk *c);

static int cgu_set_rate(struct clk *clk, unsigned long rate);
static int com_set_rate(struct clk *clk, unsigned long rate);
static unsigned long com_get_rate(struct clk *clk);

static struct clk_ops cgu_ops = {
		.set_power = com_set_power,
		.is_power_on = com_is_power_on,
		.is_enabled = com_is_enabled,

		.enable = cgu_enable,
		.disable = com_disable,

		.get_rate = com_get_rate,
		.set_rate = cgu_set_rate,

		.get_parent = com_get_parent,
		.set_parent = com_set_parent,
};

static struct clk_ops com_ops = {
		.set_power = com_set_power,
		.is_power_on = com_is_power_on,
		.is_enabled = com_is_enabled,

		.enable = com_enable,
		.disable = com_disable,

		.get_rate = com_get_rate,
		.set_rate = com_set_rate,

		.get_parent = com_get_parent,
		.set_parent = com_set_parent,
};

#define GP_VAL(a)         (((a) >> 8) & (0xff))
#define GC_VAL(a)         ((a) & (0xff))
#define PC_SET(pow, cgr)  (((pow) << 8) | (cgr))

#define P_MASK   0xff
#define C_MASK   0xff
#define PC_MASK  PC_SET(P_MASK, C_MASK)

#define CR_TYP(a)          (((a) >> 8) & (0xff))
#define CR_DIV(a)          ((a) & (0xff))
#define CR_SET(type, div)  (((type) << 8) | (div))

#define TYP_MASK   0xff
#define DIV_MASK   0xff
#define CR_MASK    CR_SET(TYP_MASK, DIV_MASK)

/* This is the CGU register definition, if there is no need to define */
#define DEF_REG_DES(n, ce, stop, div_mul, div_width, ext, ext_list, busy)   \
		struct reg_description n##_reg_des = {                              \
			offsetof(struct jz4775_cpm_regs, n##cdr),                       \
				ce, stop, div_mul, div_width, ext, ext_list, busy}

#define PAR_SET(val, typ)  (((val) << 8) | (typ))
#define PAR_MASK           PAR_SET(0xff, 0xff)
#define PAR_TYP(a)         ((a) & (0xff))
#define PAR_VAL(a)         (((a) >> 8) & (0xff))

u32 msc0_list[] = {PAR_SET(1, CLK_SCLKA), PAR_SET(2, CLK_MPLL), PAR_MASK};
DEF_REG_DES(msc0, 29, 27, 2, 8, 30, msc0_list, 28);     /* MPLL -> msc0 */
DEF_REG_DES(msc1, 29, 27, 2, 8, 30, msc0_list, 28);     /* MPLL -> msc1 */
DEF_REG_DES(msc2, 29, 27, 2, 8, 30, msc0_list, 28);     /* MPLL -> msc2 */

u32 bch_list[] = {PAR_SET(1, CLK_SCLKA), PAR_SET(2, CLK_MPLL), PAR_MASK};
DEF_REG_DES(bch, 29, 27, 1, 4, 30, bch_list, 28);       /* MPLL -> bch */

u32 ddr_list[] = {PAR_SET(1, CLK_SCLKA), PAR_SET(2, CLK_MPLL), PAR_MASK};
DEF_REG_DES(ddr, 29, 27, 1, 4, 30, ddr_list, 28);       /* MPLL -> ddr */

u32 lcd_list[] = {PAR_SET(0, CLK_SCLKA), PAR_SET(1, CLK_MPLL), PAR_MASK};
DEF_REG_DES(lcd, 28, 26, 1, 8, 30, lcd_list, 27);      /* VPLL -> lcd */

static struct clk clk_def[END_ALL] = {
		/*
		 * ext0 -> 24Mhz
		 * ext1 -> RTC
		 *  */
		[CLK_EXT0] = {"ext0", PC_MASK, 0, &ext_ops, NULL},
		[CLK_EXT1] = {"ext1", PC_MASK, 0, &ext_ops, NULL},
		[CLK_APLL] = {"apll", PC_MASK, CLK_EXT0, &pll_ops, NULL},
		[CLK_MPLL] = {"mpll", PC_MASK, CLK_EXT0, &pll_ops, NULL},
		/*
		 * NOTE: define in SPL pll.c
		 *       SCLKA CCLK L2CLK H0CLK H2CLK PCLK
		 *       can not be changed in the uboot
		 *
		 * DIV1 -> CLK_SCLKA (sclk_a)
		 *
		 *         __ CLK_CCLK (cpu)
		 * DIV2 __/__ CLK_L2CLK (cache)
		 *
		 * DIV3 -> CLK_H0CLK (AHB0)
		 *
		 *         __ CLK_H2CLK (AHB2)
		 * DIV4 __/__ CLK_PCLK (PCLK, APB)
		 *
		 * */
		[CLK_SCLKA] = {"sclka", CR_SET(30, DIV_MASK), CLK_APLL, &cr_ops, NULL},
		[CLK_CCLK] = {"cclk", CR_SET(28, 0), CLK_SCLKA, &cr_ops, NULL},
		[CLK_L2CLK] = {"l2clk", CR_SET(28, 4), CLK_SCLKA, &cr_ops, NULL},
		[CLK_H0CLK] = {"h0clk", CR_SET(26, 8), CLK_SCLKA, &cr_ops, NULL},
		[CLK_H2CLK] = {"h2clk", CR_SET(24, 12),CLK_SCLKA, &cr_ops, NULL},
		[CLK_PCLK] = {"pclk", CR_SET(24, 16), CLK_H2CLK, &cr_ops, NULL},

		/* msc<0,1,2> sele APLL
		 *
		 * MPLL __P__  CLK_MSC0 // msc1,2 the same as msc0 parent clk
		 *        \__  CLK_MSC1
		 *         \__ CLK_MSC2
		 * */
		[CLK_MSC0] = {"msc0", PC_SET(P_MASK, 3), CLK_SCLKA, &cgu_ops, &msc0_reg_des},
		[CLK_MSC1] = {"msc1", PC_SET(P_MASK, 11), CLK_SCLKA, &cgu_ops, &msc1_reg_des},
		[CLK_MSC2] = {"msc2", PC_SET(P_MASK, 12), CLK_SCLKA, &cgu_ops, &msc2_reg_des},

		/* nemc bch
		 *
		 * CLK_H2CLK (AHB2) -> nemc
		 *
		 * MPLL -> bch
		 * */
		[CLK_NEMC] = {"nemc", PC_SET(P_MASK, 0), CLK_H2CLK, &com_ops, NULL},
		[CLK_BCH] = {"bch", PC_SET(P_MASK, 1), CLK_SCLKA, &cgu_ops, &bch_reg_des},

		/* ddr0,1
		 *
		 * MPLL ____  CLK_DDR
		 * */
		[CLK_DDR] = {"ddr", PC_SET(P_MASK, 31), CLK_SCLKA, &cgu_ops, &ddr_reg_des},

		/*
		 * MPLL __G__  CLK_LCD // lcd use the same as clk gate
		 * */
		[CLK_LCD] = {"lcd", PC_SET(P_MASK, 25), CLK_SCLKA, &cgu_ops, &lcd_reg_des},
		/*              __ uart0
		 *             /__ uart1
		 * CLK_EXT0 __/___ uart2
		 *            \___ uart3
		 * */
		[CLK_UART0] = {"uart0", PC_SET(P_MASK, 15), CLK_EXT0, &com_ops, NULL},
		[CLK_UART1] = {"uart1", PC_SET(P_MASK, 16), CLK_EXT0, &com_ops, NULL},
		[CLK_UART2] = {"uart2", PC_SET(P_MASK, 17), CLK_EXT0, &com_ops, NULL},
		[CLK_UART3] = {"uart3", PC_SET(P_MASK, 18), CLK_EXT0, &com_ops, NULL},

		[CLK_PDMA] = {"pdma", PC_SET(P_MASK, 20), CLK_PCLK, &com_ops, NULL},

		[CLK_SADC] = {"sadc", PC_SET(P_MASK, 14), CLK_EXT0, &com_ops, NULL},

		[CLK_OTG] = {"otg1", PC_SET(P_MASK, 2), CLK_EXT0, &com_ops, NULL},
};

static int ext_is_enabled(struct clk *c)
{
	return true;
}

static unsigned long ext_get_rate(struct clk *c)
{
	u32 clk_type = ((u32)c - (u32)clk_def) / sizeof(struct clk);

	if(!c || clk_type > END_ALL)
		return -EOVERFLOW;

	switch(clk_type) {
	case CLK_EXT0:
		return (misc_param.extal_clock * 1000000);
	case CLK_EXT1:
		return 32768;
	default:
		printf("PLL type error, pll type:%d\n", clk_type);
		return 0;
	}

	return 0;
}

static int pll_is_enabled(struct clk *c)
{
	u32 clk_type = ((u32)c - (u32)clk_def) / sizeof(struct clk);
	struct clk *p;
	u32 ret;

	if(!c || clk_type > END_ALL)
		return -EOVERFLOW;

	if(c->parent) {
		p = &clk_def[c->parent];
		if(!p->ops->is_enabled(p)) {
			printf("%s's parent clock must be enable first.", c->name);
			return false;
		}
	}

	switch(clk_type) {
	case CLK_APLL:
		ret = (readl(&cpm_reg->cpapcr) & (1 << 10)) ? true : false;
		break;
	case CLK_MPLL:
		ret = (readl(&cpm_reg->cpmpcr) & (1 << 0)) ? true : false;
		break;
	default:
		printf("PLL type error, pll type:%d\n", clk_type);
		return -EINVAL;
	}

	return ret;
}

static unsigned long pll_get_rate(struct clk *clk)
{
	u32 clk_type = ((u32)clk - (u32)clk_def) / sizeof(struct clk);
	return get_pll_rate(clk_type);
}

static unsigned long get_pll_rate(u32 pll_mode)
{
	u32 tmp;
	unsigned long ret_rate =
			(misc_param.extal_clock * 1000000);

	switch(pll_mode) {
	case CLK_EXT0:
		return ret_rate;
	case CLK_APLL:
		tmp = readl(&cpm_reg->cpapcr);
		break;
	case CLK_MPLL:
		tmp = readl(&cpm_reg->cpmpcr);
		break;
	default:
		printf("PLL type error, pll type:%d\n", pll_mode);
		return 0;
	}

	/* rate = ((EXTCLK / (OD * N)) * M) */
	ret_rate = ((misc_param.extal_clock * 1000000)
					/ ((((tmp >> CPXPCR_PLLOD_BIT) + 1) & 0x3) *
							(((tmp >> CPXPCR_PLLN_BIT) + 1) & 0x1f))) *
							(((tmp >> CPXPCR_PLLM_BIT) + 1) & 0x7f);


	return ret_rate;
}

static u32 get_sclka_type(u32 typ)
{
	u32 ret;

	switch(typ) {
	case 0:
		ret = 0;
		break;
	case SCLKA_APLL:
		ret = CLK_APLL;
		break;
	case SCLKA_EXT0:
		ret = CLK_EXT0;
		break;
	case SCLKA_EXT1:
		ret = CLK_EXT1;
		break;
	default:
		printf("SCLKA type error %d\n", typ);
		return 0;
	}

	return ret;
}

static u32 get_cpll_type(u32 typ)
{
	u32 ret;

	switch(typ) {
	case 0:
		ret = 0;
		break;
	case CPLL_SCLKA:
		ret = CLK_SCLKA;
		break;
	case CPLL_MPLL:
		ret = CLK_MPLL;
		break;
	default:
		printf("CPLL type error %d\n", typ);
		return 0;
	}

	return ret;
}

static u32 get_h2pll_type(u32 typ)
{
	u32 ret;

	switch(typ) {
	case 0:
		ret = 0;
		break;
	case H2PLL_SCLKA:
		ret = CLK_SCLKA;
		break;
	case H2PLL_MPLL:
		ret = CLK_MPLL;
		break;
	default:
		printf("H2PLL type error %d\n", typ);
		return 0;
	}

	return ret;
}

static u32 get_pll_type(u32 clk_type, u32 typ)
{
	u32 ret = 0;

	switch(clk_type) {
	case CLK_SCLKA:
		ret = get_sclka_type(typ);
		break;
	case CLK_CCLK:
	case CLK_L2CLK:
	case CLK_H0CLK:
		ret = get_cpll_type(typ);
		break;
	case CLK_H2CLK:
	case CLK_PCLK:
		ret = get_h2pll_type(typ);
		break;
	default:
		return 0;
	}

	return ret;
}

static int cr_is_enabled(struct clk *c)
{
	u32 clk_type = ((u32)c - (u32)clk_def) / sizeof(struct clk);
	struct clk *p;
	u32 rcp;

	if(!c || clk_type > END_ALL)
		return -EOVERFLOW;

	rcp = readl(&cpm_reg->cpccr);

	clk_type = get_pll_type(clk_type, ((rcp >> CR_TYP(c->pow_cgr)) & 3));

	if(!clk_type)
		return false;

	p = &clk_def[clk_type];

	return clk_is_enabled(p);
}

static unsigned long cr_get_rate(struct clk *c)
{
	u32 clk_type = ((u32)c - (u32)clk_def) / sizeof(struct clk);
	struct clk *p;
	u32 rate_div;
	u32 rate;
	u32 rcp;

	if(!c || clk_type > END_ALL)
		return -EOVERFLOW;

	rcp = readl(&cpm_reg->cpccr);

	clk_type = get_pll_type(clk_type, ((rcp >> CR_TYP(c->pow_cgr)) & 3));

	if(!clk_type || clk_type > END_ALL) {
		printf("%s can not find the parent clock\n", c->name);
		return 0;
	}

	c->parent = clk_type;

	p = &clk_def[clk_type];

	if(clk_is_enabled(p) != true) {
		printf("%s parent [%s] clock disable\n", c->name, p->name);
		return 0;
	}

	rate = clk_get_rate(p);
	if(CR_DIV(c->pow_cgr) == DIV_MASK) {
		return rate;
	}

	rate_div = ((rcp >> CR_DIV(c->pow_cgr)) & 0xf ) + 1;

	/* debug("%s parent [%s] clock %d %d\n", c->name, p->name, rate, rate_div); */

	return rate / rate_div;
}

static int com_set_power(struct clk *clk, u32 on)
{
	u32 clk_type = ((u32)clk - (u32)clk_def) / sizeof(struct clk);
	u32 tmp;
	u32 pow_val;

	if(!clk || clk_type > END_ALL)
		return -EOVERFLOW;

	pow_val = GP_VAL(clk->pow_cgr);
	if(pow_val == P_MASK)
		return 0;

	if(pow_val > 31) {
		if(on) {
			tmp = readl(&cpm_reg->spcr1) & ~(1 << (pow_val - 32));
		} else {
			tmp = readl(&cpm_reg->spcr1) | (1 << (pow_val - 32));
		}
		writel(tmp, &cpm_reg->spcr1);
	} else {
		if(on) {
			tmp = readl(&cpm_reg->spcr0) & ~(1 << pow_val);
		} else {
			tmp = readl(&cpm_reg->spcr0) | (1 << pow_val);
		}
		writel(tmp, &cpm_reg->spcr0);
	}

	return 0;
}

static int com_is_power_on(struct clk *clk)
{
	u32 clk_type = ((u32)clk - (u32)clk_def) / sizeof(struct clk);
	u32 pow_val;
	u32 ret;

	if(!clk || clk_type > END_ALL)
		return -EOVERFLOW;

	pow_val = GP_VAL(clk->pow_cgr);
	if(pow_val == P_MASK)
		return true;
	if(pow_val > 31) {
		ret = readb(&cpm_reg->spcr1) & (1 << (pow_val -32));
	} else {
		ret = readl(&cpm_reg->spcr0) & (1 << pow_val);
	}

	ret = ret ? false : true;
	/* return 1 -> on |0 -> off */
	return ret;
}

static int com_enable(struct clk *clk)
{
	u32 clk_type = ((u32)clk - (u32)clk_def) / sizeof(struct clk);
	struct clk *pclk;
	u32 cgr_val;
	u32 tmp;

	if(!clk || clk_type > END_ALL)
		return -EOVERFLOW;

	if(!clk->ops->is_power_on(clk))
		printf("%s to auto turn on the power\n", clk->name);
	clk->ops->set_power(clk, 1);

	if(clk->parent) {
		/* if have parent clk, check it before */
		pclk = &clk_def[clk->parent];
		if(!pclk->ops->is_enabled(pclk))
			pclk->ops->enable(pclk);
	}

	cgr_val = GC_VAL(clk->pow_cgr);
	if(cgr_val == C_MASK)
		return 0;

	tmp = readl(&cpm_reg->clkgr0) & (~(1 << cgr_val));
	writel(tmp, &cpm_reg->clkgr0);


	return 0;
}

static int com_disable(struct clk *clk)
{
	u32 clk_type = ((u32)clk - (u32)clk_def) / sizeof(struct clk);
	u32 cgr_val;
	u32 tmp;

	if(!clk || clk_type > END_ALL)
		return -EOVERFLOW;

	if(!clk->ops->is_enabled(clk))
		return 0;

	cgr_val = GC_VAL(clk->pow_cgr);
	if(cgr_val == C_MASK)
		return 0;

	tmp = readl(&cpm_reg->clkgr0) | (1 << cgr_val);
	writel(tmp, &cpm_reg->clkgr0);

	return 0;
}

static int com_is_enabled(struct clk *clk)
{
	u32 clk_type = ((u32)clk - (u32)clk_def) / sizeof(struct clk);
	u32 cgr_val;
	u32 ret;

	if(!clk || clk_type > END_ALL)
		return -EOVERFLOW;

	ret = clk->ops->is_power_on(clk);
	if(!ret) {
		printf("Controller must be power on.");
		return false;
	}

	cgr_val = GC_VAL(clk->pow_cgr);
	if(cgr_val == C_MASK)
		return true;		/* if clk haven't cgr direct return 1 */

	ret = readl(&cpm_reg->clkgr0) & (1 << cgr_val);

	ret = ret ? false : true;
	return ret;
}

static int com_set_rate(struct clk *clk, unsigned long rate)
{
	return 0;
}

static int cgu_enable_change (struct clk *clk)
{
	u32 tmp;
	u32 time_out;

	struct reg_description *reg_des = (struct reg_description *)clk->reg_des;
	if (!reg_des)
		return 0;

	tmp = readl((u32)cpm_reg + reg_des->reg_offset);
	writel(tmp | (1 << reg_des->ce), (u32)cpm_reg + reg_des->reg_offset);

	if(tmp & (1 << reg_des->stop))
		return 0;

	time_out = (u32)get_timer(0);

	while((u32)get_timer(time_out) < 500 &&
			(readl((u32)cpm_reg + reg_des->reg_offset) & (1 << reg_des->busy)));
	if((u32)get_timer(time_out) > 500)
		return -ETIMEDOUT;
	else
		return 0;
}

static int cgu_enable(struct clk *clk)
{
	int ret = com_enable(clk);
	if (ret != 0)
		return ret;

	return cgu_enable_change (clk);
}

static u32 cgu_get_ext_typ(struct clk *c, struct clk *p)
{
	u32 c_type = ((u32)c - (u32)clk_def) / sizeof(struct clk);
	u32 p_type = ((u32)p - (u32)clk_def) / sizeof(struct clk);
	u32 *pt;

	if(!c || c_type > END_ALL ||
			!p || p_type > END_ALL )
		return -EOVERFLOW;

	pt = c->reg_des->ext_list;

	for(; *pt != PAR_MASK; pt ++) {
		if(PAR_TYP(*pt) == p_type)
			return PAR_VAL(*pt);
	}

	return 0;
}

static int cgu_set_rate(struct clk *clk, unsigned long rate)
{
	u32 clk_type = ((u32)clk - (u32)clk_def) / sizeof(struct clk);
	struct reg_description *reg_des;
	struct clk *pclk;
	u32 ext_type;
	u32 reg_div;
	u32 sele_clk;
	u32 tmp;

	if(!clk || clk_type > END_ALL)
		return -EOVERFLOW;

	if(!clk->ops->is_power_on(clk))
		printf("%s to auto turn on the power\n", clk->name);
	clk->ops->set_power(clk, 1);

	if(!clk->parent) {
		printf("%s does not have a parent clock\n", clk->name);
		return -EINVAL;
	}

	pclk = &clk_def[clk->parent];
	sele_clk = clk_get_rate(pclk);

	ext_type = cgu_get_ext_typ(clk, pclk);
	if(!ext_type) {
		printf("%s clock is not support %s parent clk\n",
				clk->name, pclk->name);
		return -EINVAL;
	}

	reg_des = (struct reg_description *)clk->reg_des;

	reg_div = (sele_clk / (reg_des->div_mul)) / rate -1;
	if(reg_div > ((1 << reg_des->div_width) - 1))
		reg_div = ((1 << reg_des->div_width) - 1);

	tmp = readl((u32)cpm_reg + reg_des->reg_offset);
	tmp &= (~((0xff << reg_des->ext) | ((1 << reg_des->div_width) - 1)));
	writel(tmp | (ext_type << reg_des->ext) | reg_div,
			(u32)cpm_reg + reg_des->reg_offset);

	return cgu_enable_change(clk);
}

static unsigned long com_get_rate(struct clk *clk)
{
	u32 clk_type = ((u32)clk - (u32)clk_def) / sizeof(struct clk);
	struct reg_description *reg_des;
	u32 sele_clk;
	u32 tmp;

	if(!clk || clk_type > END_ALL)
		return -EOVERFLOW;

	if(!clk_is_enabled(clk))
		return 0;

	if(!clk->parent) {
		printf("%s does not have a clock source\n", clk->name);
		return -EINVAL;
	}

	sele_clk = clk_get_rate(&clk_def[clk->parent]);

	if(!clk->reg_des) {
		return sele_clk;
	}

	reg_des = (struct reg_description *)clk->reg_des;

	tmp = readl((u32)cpm_reg + reg_des->reg_offset);
	tmp &= ((1 << reg_des->div_width) - 1);

	return (sele_clk / (reg_des->div_mul)) / (tmp + 1);
}

static struct clk *com_get_parent(struct clk *c)
{
	u32 clk_type = ((u32)c - (u32)clk_def) / sizeof(struct clk);

	if(!c || clk_type > END_ALL)
		return ERR_PTR(-EOVERFLOW);

	if(!c->parent) {
		printf("%s does not have a clock source\n", c->name);
		return ERR_PTR(-EINVAL);
	}

	return &clk_def[c->parent];
}

static int com_set_parent(struct clk *c, struct clk *p)
{
	u32 c_type = ((u32)c - (u32)clk_def) / sizeof(struct clk);
	u32 p_type = ((u32)c - (u32)clk_def) / sizeof(struct clk);
	u32 rate;

	if(!c || c_type > END_ALL ||
			!p || p_type > END_ALL)
		return -EOVERFLOW;

	if(!c->reg_des) {
		printf("%s does not set clock source\n", c->name);
		return -EINVAL;
	}

	if(c_type == CLK_MSC1 || c_type == CLK_MSC2) {
		printf("%s using the same clock source with MSC0\n", c->name);
		return 0;
	}

	rate = clk_get_rate(c);

	c->parent = p_type;

	clk_set_rate(c, rate);

	if(c_type == CLK_MSC0) {
		clk_def[CLK_MSC1].parent = clk_def[CLK_MSC0].parent;
		clk_def[CLK_MSC2].parent = clk_def[CLK_MSC0].parent;
	}

	return 0;
}

struct clk *clk_get(const char *dev_id)
{
	struct clk *pclk = clk_def;
	u32 i;

	if(!dev_id)
		return ERR_PTR(-ENODATA);

	/* Find the correct struct clk for the device and connection ID. */
	for(i = 0; i < END_ALL; i++, pclk++) {
		if (!pclk->name)
			continue;
		if(!strcmp(dev_id, pclk->name)) {
			return pclk;
		}
	}

	printf("can't find clk %s\n", dev_id);
	return ERR_PTR(-ENODATA);
}

void clk_put(struct clk *clk)
{
	return;
}

int clk_enable(struct clk *clk)
{
	if(!clk) {
		printf("clk enable error, No data available!\n");
		return -ENODATA;
	}

	if(!clk->ops->enable) {
		printf("%s do not need to enable\n", clk->name);
		return 0;
	}

	return clk->ops->enable(clk);
}

int clk_is_enabled(struct clk *clk)
{
	if(!clk) {
		printf("clk enable error, No data available!\n");
		return -ENODATA;
	}

	if(!clk->ops->is_enabled) {
		printf("%s do not need to is_enabled()\n", clk->name);
		return 0;
	}

	return clk->ops->is_enabled(clk);
}

int clk_disable(struct clk *clk)
{
	if(!clk) {
		printf("clk disable error, No data available!\n");
		return -ENODATA;
	}

	if(!clk->ops->disable) {
		printf("%s can not need to disable\n", clk->name);
		return 0;
	}

	return clk->ops->disable(clk);
}

int clk_set_rate(struct clk *clk, unsigned long rate)
{
	if(!clk) {
		printf("clk set rate error, No data available!\n");
		return -ENODATA;
	}

	if(!clk->ops->set_rate) {
		printf("%s can not need to set rate\n", clk->name);
		return 0;
	}

	return clk->ops->set_rate(clk, rate);
}

unsigned long clk_get_rate(struct clk *clk)
{
	if(!clk) {
		printf("clk get_rate error, No data available!\n");
		return -ENODATA;
	}

	if(!clk->ops->get_rate) {
		printf("%s nothing to do get_rate\n", clk->name);
		return 0;
	}

	return clk->ops->get_rate(clk);
}

int clk_set_parent(struct clk *c, struct clk *p)
{
	if(!c || !p) {
		printf("clk get_rate error, No data available!\n");
		return -ENODATA;
	}
	if(!c->ops->set_parent) {
		printf("%s nothing to do set_parent\n", c->name);
		return 0;
	}

	return c->ops->set_parent(c, p);
}

struct clk *clk_get_parent(struct clk *clk)
{
	if(!clk) {
		printf("clk get_rate error, No data available!\n");
		return ERR_PTR(-ENODATA);
	}

	if(!clk->ops->get_parent) {
		printf("%s nothing to do get_parent\n", clk->name);
		return 0;
	}

	return clk->ops->get_parent(clk);
}

int show_clk_rate(u32 aflag)
{
	u32 i;
	struct clk *pclk = clk_def;

	printf("name\t status\t parent\t rate");
	if(aflag)
		printf("\t\t addr\t value");

	/* Find the correct struct clk for the device and connection ID. */
	for(i = 0; i < END_ALL; i++, pclk++) {

		if(!pclk->name)
			continue;

		if(pclk->ops->is_enabled(pclk)) {
			printf("\n%s\t E\t %s\t %d", pclk->name,
					clk_def[pclk->parent].name, (u32)(pclk->ops->get_rate(pclk)));
			if(aflag && pclk->reg_des) {
				struct reg_description *reg_des =
					(struct reg_description *)pclk->reg_des;
				u32 addr = (u32)cpm_reg + reg_des->reg_offset;
				u32 data = readl(addr);
				printf("\t%x  %x", addr, data);
			}
		} else {
			printf("\n%s\t D", pclk->name);
		}
	}

	printf("\n");
	return 0;
}

static void init_clk_parent(void)
{
	u32 i;
	u32 *p;
	u32 tmp;

	struct clk *pclk = clk_def;
	struct reg_description *reg_des;

	/* Find the correct struct clk for the device and connection ID. */
	for(i = 0; i < END_ALL; i++, pclk++) {
		if(pclk->reg_des) {
			reg_des = (struct reg_description *)pclk->reg_des;
			tmp = readl((u32)cpm_reg + reg_des->reg_offset);
			tmp = (tmp >> reg_des->ext) & 3;
			p = reg_des->ext_list;

			if(i == CLK_MSC1 || i == CLK_MSC2) {
				pclk->parent = clk_def[CLK_MSC0].parent;
				goto NEXT;
			}

			for(; *p != PAR_MASK; p++) {
				if(PAR_VAL(*p) == tmp) {
					pclk->parent = PAR_TYP(*p);
					goto NEXT;
				}
			}
		}
		NEXT:{}
	}
}

int jz4780_clk_init(void)
{
	init_clk_parent();
	printf("M200 Soc CLK init func ;-)\n");
	return 0;
}

static int do_dclk(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 aflag;

	if (argc == 2 && (strcmp(argv[1], "all") == 0)) {
		aflag = 1;
	} else if(argc == 1)
		aflag = 0;
	else
		return CMD_RET_USAGE;

	show_clk_rate(aflag);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	clkinfo, 6, 1, do_dclk,
	"show all clk rate",
	"[all]\n"
);
