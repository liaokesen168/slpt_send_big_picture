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

#include <command.h>
#include <slpt_app.h>
#include <slpt_irq.h>
#include "slpt_gpio.h"

#if defined(CONFIG_JZ4780) ||  defined(CONFIG_JZ4775)
#include <asm/arch/jz4775_gpio.h>
#elif defined(CONFIG_M200)
#include <asm/arch/m200_gpio.h>
#endif

#include "slpt_app_alarm.h"

extern void _machine_restart(void);

static void slpt_powerkey_handler(unsigned int irq, struct irq_desc *desc)
{
	debug("irq call %s\n", __func__);

	slpt_mode_exit();

	/* clean PA30(powerley) irq pending bit */

	/*
	 * REG_GPIO_PXINTC(0) = 1 << 30;
	 * REG_GPIO_PXMASKS(0) = 1 << 30;
	 * */
}

static int slpt_powerkey_init(void) {
	debug("init call %s\n", __func__);

	slpt_enable_gpio_interrupt(SLPT_GPIO_PA(30), slpt_powerkey_handler, TRIGGLE_FALL);

	return 0;
}

SLPT_APP_INIT_ONETIME(slpt_powerkey_init);
