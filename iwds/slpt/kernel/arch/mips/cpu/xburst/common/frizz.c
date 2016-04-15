#include <common.h>
#include <slpt_irq.h>
#include "slpt_gpio.h"
#include <slpt_app.h>
#include <frizz.h>
#include <slpt.h>

extern unsigned long pedo_value;
extern unsigned long sleep_motion_enable;

static void frizz_irq_handler(unsigned int irq, struct irq_desc *desc) {
	read_fifo();
}


static int frizz_irq_init(void) {
	int status = -1;
	status = frizz_i2c_probe();
	if(!status) {
		frizz_i2c_transfer_test();
		SLPT_GET_KEY(SLPT_K_FRIZZ_PEDO, &pedo_value);
		SLPT_GET_KEY(SLPT_K_SLEEP_MOTION_ENABLE, &sleep_motion_enable);
		slpt_enable_gpio_interrupt(SLPT_GPIO_PA(9), frizz_irq_handler, TRIGGLE_FALL);
		slpt_kernel_printf("slpt-frizz probe success, gpio : %d, pedo_data: %d \n", SLPT_GPIO_PA(9), pedo_value);
	} else {
		slpt_kernel_printf("%s --frizz_i2c probe failed!!!!!!\n", __func__);
	}
	return 0;
}

SLPT_APP_INIT_EVERYTIME(frizz_irq_init);
