#ifndef _GPIO_DESC_T_H_
#define _GPIO_DESC_T_H_

#include <linux/lcd_info_debug.h>
#include <asm-generic/gpio.h>

/* Sorry for GPIO_PA0, for default it is not define the gpio if the gpio num is 0 */
#define gpio_valid(gpio) ((gpio) > 0)

#define if_gpio_direction_output(gpio, value)	\
	do {                                        \
		if (gpio_valid(gpio))					\
			gpio_direction_output(gpio, value);	\
	} while (0)

#define if_gpio_request(gpio, label) (gpio_valid(gpio) ? gpio_request(gpio, label) : 0)
#define if_gpio_free(gpio)						\
	do {                                        \
		if (gpio_valid(gpio))					\
			gpio_free(gpio);                    \
	} while (0)

typedef struct gpio_desc_t {
	int gpio;
	int valid_level;
} gpio_desc_t;

#define gpio_desc_valid(gpio_desc) gpio_valid((gpio_desc)->gpio)

#define gpio_desc_direction_output(gpio_desc, value) gpio_direction_output((gpio_desc)->gpio, value)
#define gpio_desc_direction_output_low(gpio_desc) gpio_direction_output((gpio_desc)->gpio, (gpio_desc)->valid_level)
#define gpio_desc_direction_output_high(gpio_desc) gpio_direction_output((gpio_desc)->gpio, !(gpio_desc)->valid_level)

#define gpio_desc_set_low(gpio_desc)									\
	do {																\
		gpio_direction_output(gpio_desc->gpio, gpio_desc->valid_level);	\
	} while (0)

#define gpio_desc_set_high(gpio_desc)									\
	do {																\
		gpio_direction_output(gpio_desc->gpio, !gpio_desc->valid_level); \
	} while (0)

#define gpio_desc_request(gpio_desc, label) (gpio_desc_valid(gpio_desc) ? gpio_request((gpio_desc)->gpio, label) : 0)

#define gpio_desc_free(gpio_desc)				\
	do {                                        \
		if (gpio_desc_valid(gpio_desc))			\
			gpio_free((gpio_desc)->gpio);		\
	} while (0)

#endif /* _GPIO_DESC_T_H_ */
