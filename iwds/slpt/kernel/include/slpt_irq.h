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

#ifndef _SLPT_IRQ_H_
#define _SLPT_IRQ_H_

/* IRQ for MIPS CPU */
#define MIPS_CPU_IRQ_BASE 		0
#define IRQ_SMP_RESCHEDULE_YOURSELF	3
#define IRQ_SMP_CALL_FUNCTION		4

#define INTC_IRQ_BASE		0

#define GPIO_GROUP 6
#define GPIO_PORTS 32

enum {
#define INTC_NR_IRQS	64
	IRQ_INTC_BASE = INTC_IRQ_BASE,
	IRQ_INTC_END = IRQ_INTC_BASE + INTC_NR_IRQS - 1,

#define GPIO_NR_IRQS	(32*5 + 2*23)
	IRQ_GPIO_BASE,
	IRQ_GPIO_END = IRQ_GPIO_BASE + GPIO_NR_IRQS - 1,

#define SADC_NR_IRQS	(6)
	IRQ_SADC_BASE,
	IRQ_SADC_END = IRQ_SADC_BASE + SADC_NR_IRQS - 1,

#define MCU_NR_IRQS	(5)
	IRQ_MCU_BASE,
	IRQ_MCU_END = IRQ_MCU_BASE + MCU_NR_IRQS - 1,
	NR_IRQS,
};

enum {
// interrupt controller interrupts
	IRQ_RESERVED0 = IRQ_INTC_BASE,
	IRQ_AIC0,
	IRQ_BCH,
	IRQ_RESERVED3,
	IRQ_RESERVED4,
	IRQ_OHCI,
	IRQ_RESERVED6,
	IRQ_RESERVED7,
	IRQ_SSI0,
	IRQ_RESERVED9,
	IRQ_PDMA,
	IRQ_GPIO6,
	IRQ_GPIO5,
	IRQ_GPIO4,
	IRQ_GPIO3,
	IRQ_GPIO2,
	IRQ_GPIO1,
	IRQ_GPIO0,
#define IRQ_GPIO_PORT(N) (IRQ_GPIO0 - (N))
	IRQ_SADC,
	IRQ_X2D,
	IRQ_RESERVED20,
	IRQ_OTG,
	IRQ_EPDCE,
	IRQ_EPDC,
	IRQ_RESERVED24,
	IRQ_TCU2,
	IRQ_TCU1,		//spec is not found
	IRQ_TCU0,
	IRQ_CIM1,
	IRQ_RESERVED29,
	IRQ_CIM,
	IRQ_LCD0,

	IRQ_RTC,
	IRQ_OWI,
	IRQ_RESERVED34,
	IRQ_MSC2,
	IRQ_MSC1,
	IRQ_MSC0,
	IRQ_RESERVED38,
	IRQ_RESERVED39,
	IRQ_PCM0,
	IRQ_KBC,
	IRQ_RESERVED42,
	IRQ_RESERVED43,
	IRQ_HARB2,
	IRQ_HARB1,
	IRQ_HARB0,
	IRQ_CPM,
	IRQ_UART3,
	IRQ_UART2,
	IRQ_UART1,
	IRQ_UART0,
	IRQ_DDR,
	IRQ_RESERVED53,
	IRQ_NEMC,
	IRQ_GMAC,
	IRQ_RESERVED56,
	IRQ_RESERVED57,
	IRQ_I2C2,
	IRQ_I2C1,
	IRQ_I2C0,
	IRQ_PDMAM,
	IRQ_VPU,
	IRQ_RESERVED063,
};

struct irq_desc;

typedef	void (*irq_flow_handler_t)(unsigned int irq, struct irq_desc *desc);

struct irq_desc {
	int irq_num;
	int mask_flag;
	irq_flow_handler_t irq_handler;
};

extern void slpt_do_irq(void);
extern void slpt_irq_check_mask(void);

extern int register_irq_handler(unsigned int irq_num, irq_flow_handler_t handler);

#endif
