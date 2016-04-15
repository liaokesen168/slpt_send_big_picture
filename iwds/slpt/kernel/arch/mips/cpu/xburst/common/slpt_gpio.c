#include "slpt_gpio.h"
#include <config.h>
#include <asm/io.h>
#include <slpt_app.h>
#include <common.h>

#if defined(CONFIG_JZ4780) ||  defined(CONFIG_JZ4775)
#include <asm/arch/jz4775_gpio.h>
#elif defined(CONFIG_M200)
#include <asm/arch/m200_gpio.h>
#endif


void dump_reg(int reg)
{
    debug("reg :0x%x = [0x%x] \n", reg, readl(reg));
}


int gpio_to_irq(int gpio)
{
    if((gpio < 0)|| (gpio > GPIO_GROUP * GPIO_PORTS - 1))
        return -1;
    return gpio + INTC_NR_IRQS;
}

int enable_gpio_interrupt_reg(int gpio, int triggle)
{
	int group;
	int pin;
	if(gpio <= 0)
	    return -1;

	group = gpio / 32;
	pin  = gpio % 32;

	debug("gpio = %d, group = %d, pin = %d \n", gpio, group, pin);
    //enabled interrupt function
	//enable the group irq
//	writel(1 << (5 - group + 12),0xB0001000 + 0xC);
	//enable the pin irq
    REG_GPIO_PXINTS(group)  = 1 << pin;
    REG_GPIO_PXMASKC(group) = 1 << pin;

	//set triggled mode
	switch(triggle) {
		case TRIGGLE_LOW:
		    REG_GPIO_PXPAT1C(group) = 1 << pin;
			REG_GPIO_PXPAT0C(group) = 1 << pin;
			break;
		case TRIGGLE_HIGH:
			REG_GPIO_PXPAT1C(group) = 1 << pin;
			REG_GPIO_PXPAT0S(group) = 1 << pin;
			break;
		case TRIGGLE_FALL:
			REG_GPIO_PXPAT1S(group) = 1 << pin;
			REG_GPIO_PXPAT0C(group) = 1 << pin;
			break;
		case TRIGGLE_RAISE:
		    REG_GPIO_PXPAT1S(group) = 1 << pin;
			REG_GPIO_PXPAT0S(group) = 1 << pin;
			break;
		default:
			return -1;
			break;
	}

	/* clear the interrupt flag, after set the function,
	   because we do not use the shadow registers here */
	REG_GPIO_PXFLGC(group) = 1 << pin;

	dump_reg(0xB0010000 + 0x100 * group + 0x10);
	dump_reg(0xB0010000 + 0x100 * group + 0x20);
	dump_reg(0xB0010000 + 0x100 * group + 0x30);
	dump_reg(0xB0010000 + 0x100 * group + 0x40);
	return 0;
}

int slpt_enable_gpio_interrupt(int gpio, irq_flow_handler_t handler, int triggle_mode)
{
	int irq;
	if(handler == 0)
		goto Error;

	irq = gpio_to_irq(gpio);
	if(irq < 0)
		goto Error;

	if (enable_gpio_interrupt_reg(gpio, triggle_mode))
		goto Error;

	return register_irq_handler(irq, handler);

Error:
	return -1;
}
