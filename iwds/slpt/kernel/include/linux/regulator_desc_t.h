#ifndef _REGULATOR_DESC_T_H_
#define _REGULATOR_DESC_T_H_

#include <linux/gpio_desc_t.h>
#include <linux/lcd_info_debug.h>
#include <regulator.h>
#include <linux/err.h>

struct device ;
typedef char* regulator_desc_id;

#define reg_valid(reg_id) ((reg_id) != 0)

typedef struct regulator_desc_t {
	regulator_desc_id id;
	unsigned int voltage;
	unsigned int voltage_inited:1;
	gpio_desc_t gpio;
	struct regulator *reg;
} regulator_desc_t;

#define reg_desc_valid(reg_desc) (reg_valid(reg_desc->id))

static inline int regulator_desc_get(struct device *dev, regulator_desc_t *reg_desc) {

	if (reg_desc_valid(reg_desc)) {
		if (reg_desc->voltage < 0xff) {
			lcd_info_tag("ERROR: Please change your voltage", __FUNCTION__);
			return -EINVAL;
		}

		reg_desc->reg = regulator_get(reg_desc->id);
		if (IS_ERR(reg_desc->reg)) {
			lcd_info_tag("Failed to request regulator\n", __FUNCTION__);
			return -EINVAL;
		}
	}
	if (gpio_desc_valid(&reg_desc->gpio)) {
		if (gpio_desc_request(&reg_desc->gpio, "regulator-gpio")) {
			lcd_info_tag("Failed to request regulator gpio\n", __FUNCTION__);
			if (reg_desc_valid(reg_desc))
				regulator_put(reg_desc->reg);
			return -EINVAL;
		}
	}
	return 0;
}

static inline void regulator_desc_put(struct regulator_desc_t *reg_desc) {
	if (gpio_desc_valid(&reg_desc->gpio))
		gpio_desc_free(&reg_desc->gpio);

	if (reg_desc_valid(reg_desc))
		regulator_put(reg_desc->reg);
}

static inline int regulator_desc_enable(regulator_desc_t *reg_desc) {

	if (reg_desc_valid(reg_desc)) {
		regulator_set_voltage(reg_desc->reg, reg_desc->voltage, reg_desc->voltage);
		if (!regulator_is_enabled(reg_desc->reg))
			regulator_enable(reg_desc->reg);
	}
	if (gpio_desc_valid(&reg_desc->gpio)) {
		gpio_desc_direction_output_low(&reg_desc->gpio);
	}
	return 0;
}

static inline int regulator_desc_disable(regulator_desc_t *reg_desc) {
	if (gpio_desc_valid(&reg_desc->gpio)) {
		gpio_desc_direction_output_high(&reg_desc->gpio);
	}
	if (reg_desc_valid(reg_desc)) {
		return regulator_disable(reg_desc->reg);
	}
	return 0;
}

#endif /* _REGULATOR_DESC_T_H_ */
