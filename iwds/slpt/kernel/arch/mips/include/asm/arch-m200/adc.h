/*
 * Copyright (C) 2013 Ingenic Semiconductor Co., Ltd.
 * Authors: Kage Shen <kkshen@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 3 of the License, or (at your option) any later version.
 */
#ifndef __M200_ADC_H__
#define __M200_ADC_H__

enum adc_channel {
	CH_VBAT = 'C',
	CH_AUX1,
	CH_AUX2,
};

extern int adc_get_value(enum adc_channel ch);
extern int jz4780_sadc_init(void);
extern int jz4780_sadc_power_off(void);

#endif
