/*
 * Ingenic GPIO lib support
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co., Ltd.
 *		Kage Shen <kkshen@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <common.h>

#include <linux/compat.h>
#include <linux/err.h>

#include <asm/io.h>
#include <asm/errno.h>
#if defined(CONFIG_JZ4780)
#include <asm/arch/jz4780_gpio.h>
#elif defined(CONFIG_JZ4775)
#include <asm/arch/jz4775_gpio.h>
#elif defined(CONFIG_M200)
#include <asm/arch/m200_gpio.h>
#endif

struct gpio_label {
	int gpio;
	const char *label;

	int in_use;
};

static struct gpio_label request_table[192];

int gpio_is_valid(int gpio)
{
	return (gpio >= 0) && (gpio < 192);
}

static int check_gpio(int gpio)
{
	if (!gpio_is_valid(gpio)) {
		printk("ERROR : check_gpio: invalid GPIO %d\n", gpio);
		return -1;
	}
	return 0;
}

/**
 * Set value of the specified gpio
 */
int gpio_set_value(int gpio, int value)
{

	if (check_gpio(gpio) < 0)
		return -1;

	if(value)
		gpio_set_pin(gpio);
	else
		gpio_clear_pin(gpio);

	return 0;
}

/**
 * Get value of the specified gpio
 */
int gpio_get_value(int gpio)
{
	u32 tmp;

	if (check_gpio(gpio) < 0)
		return -1;
	tmp = gpio_get_pin(gpio);

	return tmp;
}

/**
 * Set gpio direction as input
 */
int gpio_direction_input(int gpio)
{
	if (check_gpio(gpio) < 0)
		return -1;

	gpio_as_input(gpio);

	return 0;
}

/**
 * Set gpio direction as output
 */
int gpio_direction_output(int gpio, int value)
{
	if (check_gpio(gpio) < 0)
		return -1;

	gpio_as_output(gpio);
	gpio_set_value(gpio, value);

	return 0;
}

/**
 * Request a gpio before using it.
 *
 * NOTE: Argument 'label' is unused.
 */
int gpio_request(int gpio, const char *label)
{
	if (check_gpio(gpio))
		return -EINVAL;

	if (request_table[gpio].in_use)
		return -EBUSY;
	else {
		request_table[gpio].in_use = 1;
		request_table[gpio].label = label;
	}

	return 0;
}

int gpio_free(int gpio)
{
	if (check_gpio(gpio))
		return -EINVAL;

	if (request_table[gpio].in_use)
		request_table[gpio].in_use = 0;
	else
		return -EIO;

	return 0;
}


int jz_gpio_set_func(int gpio, enum gpio_function func)
{
	switch (func) {
	case GPIO_FUNC_0:
		gpio_as_func0(gpio);
		break;

	case GPIO_FUNC_1:
		gpio_as_func1(gpio);
		break;

	case GPIO_FUNC_2:
		gpio_as_func2(gpio);
		break;

	case GPIO_FUNC_3:
		gpio_as_func3(gpio);
		break;
	}

	return 0;
}

static int
gpio_debugfs_show(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i;
	int index;
	int group;

	for (i = 0; i < 192; i++) {
		if (!request_table[i].in_use)
			continue;

		index = i & 0x1f;
		group = i >> 5;

		printk("GPIO-%c-%d: label: %s, value: %s\n", 'A' + group, index,
				request_table[i].label ? request_table[i].label : "",
						gpio_get_value(i) ? "Hi" : "Lo");
	}

	return 0;
}

U_BOOT_CMD(
	gpioinfo, 1, 1, gpio_debugfs_show,
	"show GPIO info",
	"cmd: gpioinfo"
);
