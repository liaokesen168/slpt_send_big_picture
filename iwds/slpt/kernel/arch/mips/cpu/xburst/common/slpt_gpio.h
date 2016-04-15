#ifndef __SLPT_GPIO_H__
#define __SLPT_GPIO_H__

#include <slpt_irq.h>

#define SLPT_GPIO_PA(x) (32 * 0 + x)
#define SLPT_GPIO_PB(x) (32 * 1 + x)
#define SLPT_GPIO_PC(x) (32 * 2 + x)
#define SLPT_GPIO_PD(x) (32 * 3 + x)
#define SLPT_GPIO_PE(x) (32 * 4 + x)
#define SLPT_GPIO_PF(x) (32 * 5 + x)

enum {
	TRIGGLE_HIGH,
	TRIGGLE_LOW,
	TRIGGLE_FALL,
	TRIGGLE_RAISE,
};

int slpt_enable_gpio_interrupt(int gpio, irq_flow_handler_t handler, int triggle_mode);
#endif
