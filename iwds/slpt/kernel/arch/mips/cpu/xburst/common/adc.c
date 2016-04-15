/*
 * Copyright (C) 2013 Ingenic Semiconductor Co., Ltd.
 * Authors: Kage Shen <kkshen@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 3 of the License, or (at your option) any later version.
 */

#include <config.h>
#include <common.h>

#include <linux/err.h>
#include <asm/io.h>
#include <asm/arch/adc.h>
#include <asm/arch/cpm.h>

struct jz4780_adc_regs {
	volatile u32 ena;   /* ADC Enable Register */
	volatile u32 cfg;   /* ADC Configure Register */
	volatile u32 ctrl;  /* ADC Control Register */
	volatile u32 state; /* ADC Status Register*/
	volatile u32 same;  /* ADC Same Point Time Register */
	volatile u32 wait;  /* ADC Wait Time Register */
	volatile u32 tch;   /* ADC Touch Screen Data Register */
	volatile u32 vdat;  /* ADC PBAT Data Register */
	volatile u32 adat;  /* ADC SADCIN Data Register */
	volatile u32 cmd;
	volatile u32 clk;   /* ADC SADCLK Register */
};

#define SADC_BASE 0xB0070000

#define SADC_AUXEN (1)
#define SADC_VBATEN (1 << 1)
#define SADC_POWER_OFF (1 << 7)


#define CPM_SPCR0  ((void*)(0xB0000000 + 0x00B8))  //memory power control for ADC
#define CPM_CLKGR0 ((void*)(0xB0000000 + 0x0020))  //Gate ADC's pclk

static struct jz4780_adc_regs *adc_reg = (struct jz4780_adc_regs *)SADC_BASE;

int jz4780_sadc_init(void)
{

#ifdef CONFIG_M200
	writel(readl(CPM_SPCR0) & (~(1<<6)), CPM_SPCR0);  //memory power control for adc
	writel(readl(CPM_CLKGR0) & (~(1<<13)), CPM_CLKGR0);  //Gate adc's pclk
#else
	struct clk *clk;

	clk = clk_get("sadc");
	if(!clk) {
		printf("sADC requst clk error\n");
		return -ENOLCK;
	}
	printf("jz4780 sADC initialized\n");
	clk_enable(clk);
#endif

	writel(readl(&adc_reg->ena) & ~SADC_POWER_OFF, &adc_reg->ena);
	mdelay(2);
	writel(0xbc02ff, &adc_reg->clk);
	return 0;
}

int jz4780_sadc_power_off(void) {
	writel(readl(&adc_reg->ena) | SADC_POWER_OFF, &adc_reg->ena);

#ifdef CONFIG_M200
	writel(readl(CPM_CLKGR0) | (1<<13), CPM_CLKGR0);  //Gate adc's pclk
	writel(readl(CPM_SPCR0) | (1<<6), CPM_SPCR0);  //memory power control for adc
#else
	{
		struct clk *clk;

		clk = clk_get("sadc");
		if(!clk) {
			printf("sADC requst clk error\n");
			return -ENOLCK;
		}
		printf("jz4780 sADC initialized\n");
		clk_disable(clk);
	}
#endif

	return 0;
}

static inline u32 adc_get_vbat(void)
{
	writel(readl(&adc_reg->ena) | SADC_VBATEN, &adc_reg->ena);

	while(!(readl(&adc_reg->state) & (1<<1)));
	writel(readl(&adc_reg->state) | (1<<1), &adc_reg->state);

	return (readl(&adc_reg->vdat) & 0xfff);
}

int adc_get_value(enum adc_channel ch)
{
	u32 ret = 0;

	switch (ch) {
	case CH_VBAT:
		ret = adc_get_vbat();
		break;
	case CH_AUX1:
	case CH_AUX2:
		break;
	default:
		printf("Can not enable the channel\n");
		return -EINVAL;
	}

	return ret;
}
