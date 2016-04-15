/*
 * Copyright (C) 2014 Ingenic Semiconductor Co., Ltd.
 * Authors: Kage Shen <kkshen@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the License, or (at your
 * option) any later version.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <config.h>
#include <common.h>
#include <malloc.h>

#include <linux/err.h>
#include <asm/io.h>

#include <slpt_app.h>
#include <slpt_irq.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

extern int low_pmu_voltage_mode(void);

static struct irq_desc irq_handler[NR_IRQS];

struct slpt_irq_param {
	unsigned int kernel_irq_mask0;
	unsigned int kernel_irq_mask1;

	unsigned int uboot_irq_mask0;
	unsigned int uboot_irq_mask1;
};

static struct slpt_irq_param irq_param;

void slpt_irq_check_mask(void)
{
	irq_param.kernel_irq_mask0 = readl(0xB0001004);
	irq_param.kernel_irq_mask1 = readl(0xB0001024);
}

static void inline do_irq_handler(unsigned int irq_num)
{
	if (irq_handler[irq_num].mask_flag != 1 &&
			irq_handler[irq_num].irq_handler)
		irq_handler[irq_num].irq_handler(irq_num, &irq_handler[irq_num]);
	else {
		slpt_mode_exit();
	}
}

void print_irq(void) {
#if 0
	slpt_kernel_printf("mask\t%x %x\n", readl(0xB0001004), readl(0xB0001024));
	slpt_kernel_printf("pending\t%x %x\n", readl(0xB0001010), readl(0xB0001030));
	slpt_kernel_printf("srouce\t%x %x\n", readl(0xB0001000), readl(0xB0001020));
#endif
}

void do_gpio_irq_handler(int group)
{
    unsigned int Pxflg;
    unsigned int Pxmask;
    unsigned int irq_num = 0;
    int i = 0;
    if(group < 0 || group > GPIO_GROUP)
        return;
    //1.do the map
    Pxflg = readl(0xB0010000 + 0x100 * group + 0x50);
    Pxmask = readl(0xB0010000 + 0x100 * group + 0x20);
    //2. no priority
    for(i = 0; i < 32; i++) {
		if((1 << i) & (Pxflg) & (~Pxmask)) {
			irq_num = INTC_NR_IRQS + GPIO_PORTS * group + i;

			if (irq_handler[irq_num].mask_flag != 1 &&
				irq_handler[irq_num].irq_handler) {
				//3. mask the irq and clean the flag, then call the handler function
				writel(1 << i, 0xB0010000 + 0x100 * group + 0x24);
				writel(1 << i, 0xB0010000 + 0x100 * group + 0x58);

				debug("irq_num = %d , i = %d \n", irq_num, i);
				do_irq_handler(irq_num);
				//unmask the irq
				writel(1 << i, 0xB0010000 + 0x100 * group + 0x28);
			} else {
				slpt_mode_exit();
			}
		}
    }
}

void slpt_do_irq(void)
{
	unsigned int icpr0, icpr1, i;
	unsigned int group = 0;
	unsigned int mask0, mask1;
	/* unsigned int slpt_mode = get_slpt_run_mode(); */

	/* if ( slpt_mode == SLPTARG_MODE_LOWPOWER || slpt_mode == SLPTARG_MODE_WARNING ) { */
	if (low_pmu_voltage_mode()) {
		/* slpt not respond other interrupt */
		writel(1 << 25, 0xB000100c);
	} else {
		mask0 = (~irq_param.kernel_irq_mask0) | irq_param.uboot_irq_mask0;
		mask1 = (~irq_param.kernel_irq_mask1) | irq_param.uboot_irq_mask1;
		writel(mask0, 0xB000100c);
		writel(mask1, 0xB000102c);
	}

	debug("mask\t%x %x\n", readl(0xB0001004), readl(0xB0001024));
	debug("pending\t%x %x\n", readl(0xB0001010), readl(0xB0001030));
	debug("srouce\t%x %x\n", readl(0xB0001000), readl(0xB0001020));

	icpr0 = readl(0xB0001010);
	if(icpr0 & (1 << 25)) {
		do_irq_handler(25);
	}
	for (i = 0; i < 32; i++) {
		icpr0 = readl(0xB0001010);
		icpr1 = readl(0xB0001030);

		if(icpr0 & (1 << i)) {
			if(i == 25)
				continue;
			/* when voice trigger is enabled, will ignore those irqs: dmic and rtc */
			if ((i == 0 || i == 26) && (slpt_kernel_get_voice_trigger_state() == 1))
				continue;
			// the gpio interrupt deal with other way
			if((1 << i) & (0x3F << 12)) {
		    // get the group
				group = 5 - ( i - 12 );
				do_gpio_irq_handler(group);
				continue;
			}
			do_irq_handler(i);
		}
		if(icpr1 & (1 << i)) {
			do_irq_handler(i + 32);
		}
	}

}

int register_irq_handler(unsigned int irq_num, irq_flow_handler_t handler)
{
	struct irq_desc *desc = &irq_handler[irq_num];

	if(desc->irq_handler && (desc->mask_flag == 0)) {
		debug("irq %d has registered\n", irq_num);
		return -EEXIST;
	} else {
		if (handler && (irq_num < INTC_NR_IRQS)) {
			desc->irq_handler = handler;
			desc->mask_flag = 0;

			if (irq_num < 32)
				irq_param.uboot_irq_mask0 |= 1 << irq_num;
			else
				irq_param.uboot_irq_mask1 |= 1 << (irq_num - 32);

			return 0;
		} else if (handler && (irq_num < GPIO_NR_IRQS)) {
		    //do the gpio-pins-irq mapping
		    desc->irq_handler = handler;
		    desc->mask_flag = 0;
		} else {
			debug("irq regidster error invalid argument\n");
			return -EINVAL;
		}
	}

	return 0;
}

static int slpt_irq_init(void)
{
	unsigned int i;

	for (i = 0; i < INTC_NR_IRQS; i++) {
		struct irq_desc *handler = &irq_handler[i];
		handler->irq_handler = NULL;
		handler->irq_num = i;
		handler->mask_flag = 1;
	}

	irq_param.uboot_irq_mask0 = 0;
	irq_param.uboot_irq_mask1 = 0;

	debug("slpt irq handler init ok\n");

	return 0;
}
SLPT_ARCH_INIT_ONETIME(slpt_irq_init);

static int slpt_irq_exit(void)
{
    //restore the kernel irq mask setting back
	writel(~irq_param.kernel_irq_mask0, 0xB000100c);
	writel(~irq_param.kernel_irq_mask1, 0xB000102c);
	return 0;
}
SLPT_APP_EXIT_EVERYTIME(slpt_irq_exit);
