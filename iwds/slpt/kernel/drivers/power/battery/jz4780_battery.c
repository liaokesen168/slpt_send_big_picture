/*
 * Battery measurement code for Ingenic JZ SoC
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co., Ltd.
 * Authors: Kage Shen <kkshen@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 3 of the License, or (at your option) any later version.
 */
#include <common.h>
#include <regulator.h>
#include <pfsv1.h>

#include <linux/err.h>
#include <asm/gpio.h>
#include <asm/arch/adc.h>
#include <asm/arch/board_special.h>
#if defined(CONFIG_JZ4780)
#include <asm/arch/jz4780_gpio.h>
#include <asm/arch/jz4780_pfsv1.h>
#elif defined(CONFIG_JZ4775)
#include <asm/arch/jz4775_gpio.h>
#include <asm/arch/jz4775_pfsv1.h>
#elif defined(CONFIG_M200)
#include <asm/arch/m200_gpio.h>
#include <asm/arch/m200_pfsv1.h>
#endif
#include <asm/arch/board_special.h>
#include <power/jz4780_battery.h>

static struct battery_dec bat_dec;

struct boot_battery_parameter {
	/* batery voltage = (value * x) / y + z */
	unsigned int para_x;
	unsigned int para_y;
	unsigned int para_z;

	unsigned int max_vol;
	unsigned int min_vol;
	unsigned int usb_max_vol;
	unsigned int usb_min_vol;
	unsigned int ac_max_vol;
	unsigned int ac_min_vol;

	unsigned int battery_max_cpt;
	unsigned int ac_chg_current;
	unsigned int usb_chg_current;
};

struct boot_battery_parameter bat_param = {
	.para_x = 1000,
	.para_y = 1,
	.para_z = 600000,
	.max_vol = 4050,
	.min_vol = 3600,
	.usb_max_vol = 4100,
	.usb_min_vol = 3760,
	.ac_max_vol = 4100,
	.ac_min_vol = 3760,
	.battery_max_cpt = 3000,
	.ac_chg_current = 800,
	.usb_chg_current = 400,
};

int get_battery_voltage(void)
{
	struct boot_battery_parameter *pdata = &bat_param;
	u32 ret = 0;
	u32 val[12];
	u32 max_val = 0, min_val = 0xfff;
	u32 i;

	for(i = 0; i < 12; i++) {
		val[i] = adc_get_value(CH_VBAT);
		ret += val[i];
		if(max_val < val[i])
			max_val = val[i];
		if(min_val > val[i])
			min_val = val[i];
	}

	ret -= (max_val + min_val);
	ret /= 10;

	return ((ret * pdata->para_x) / pdata->para_y + pdata->para_z);
}

int jz4780_battery_init(void)
{
	int ret = 0;
	struct regulator *reg;

	if(!bat_dec.ops) {
		printf("board haven't battery charger\n");
		return -ENXIO;
	}

	reg = regulator_get("charger");
	if(reg <= 0) {
		printf("Can not get 'charger' regulator");
		return ret;
	}

	bat_dec.name = "Li-battery";
	bat_dec.reg = reg;

	bat_dec.ops->charger_enable(reg);

	/* set AC charger current to 800mA */
	/* gpio_direction_output(GPIO_PB(2), 0); */

	ret = bat_dec.ops->get_charger_status();
	switch(ret) {
	case CHARGER_AC:
		debug("AC is connected.\n");
		break;
	case CHARGER_USB:
		debug("USB is connected.\n");
		break;
	default:
		debug("%s in use\n", bat_dec.name);
		break;
	}

	printf("board battary initialized. Vb: %d uV\n",
			get_battery_voltage());

	return 0;
}

int jz4780_battery_cherger(struct charger_ops *ops)
{
	if(ops == NULL)
		return -EINVAL;
	bat_dec.ops = ops;
	return 0;
}

static int show_battery_info(void)
{
	struct boot_battery_parameter *pdata = &bat_param;
	u32 vol = get_battery_voltage() / 1000;

	printf("%s battery information\nCurrent voltage %d mV\n",
			bat_dec.name, vol);

	printf("\tmax_vol is %d mV\n", pdata->max_vol);
	printf("\tmin_vol is %d\n mV", pdata->min_vol);
	printf("\tusb_max_vol is %d mV\n", pdata->usb_max_vol);
	printf("\tusb_min_vol is %d mV\n", pdata->usb_min_vol);
	printf("\tac_max_vol is %d mV\n", pdata->ac_max_vol);
	printf("\tac_min_vol is %d mV\n", pdata->ac_min_vol);
	printf("\tbattery_max_cpt is %d mV\n", pdata->battery_max_cpt);
	printf("\tac_chg_current is %d mA\n", pdata->ac_chg_current);
	printf("\tusb_chg_current is %d mA\n", pdata->usb_chg_current);

	return CMD_RET_SUCCESS;
}

static int battery_check_vol(void)
{
	struct boot_battery_parameter *pdata = &bat_param;
	u32 vol;
	u32 ret;
	u32 wait_time;

CHECK_AGAIN:
	vol = get_battery_voltage() / 1000;
	ret = bat_dec.ops->get_charger_status();
	switch(ret) {
	case CHARGER_AC:
		if(vol > pdata->ac_max_vol)
			regulator_disable(bat_dec.reg);
		break;
	case CHARGER_USB:
		if(vol > pdata->usb_max_vol)
			regulator_disable(bat_dec.reg);
		else if(vol < pdata->usb_min_vol) {
			regulator_enable(bat_dec.reg);
			regulator_set_current_limit(bat_dec.reg,
					pdata->usb_chg_current * 1000,
					pdata->usb_chg_current * 1000);
			goto NEXT;
		}
		break;
	default:
		if(vol < pdata->min_vol) {
			goto NEXT;
		}
		break;
	}
	return CMD_RET_SUCCESS;

NEXT:
	debug("%s battery voltage low, Vb: %d mV\n", bat_dec.name, vol);
	wait_time = get_timer(0);
	while(get_timer(wait_time) < 500);
	goto CHECK_AGAIN;

	return 0;
}

static int do_battery(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc < 2)
		return CMD_RET_USAGE;

	if (strcmp(argv[1], "info") == 0) {
		if (argc != 2)
			return CMD_RET_USAGE;
		return show_battery_info();
	} else if (strcmp(argv[1], "check") == 0) {
		if (argc != 2)
			return CMD_RET_USAGE;
		return battery_check_vol();
	}

	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	battery, 2, 1, do_battery,
	"Battery ops",
	"info - show battery information\n"
	"battery check - check battery low voltage"
);

PFSV1_DEF_PARAM_BEGIN(Battery, "Battery subsystem",
        "text: Battery configuration")
    PFSV1_DEF_PARAM_BLOCK_BEGIN(bat_param, "Configuration",
            struct boot_battery_parameter,
            "text: Battery configuration")
        PFSV1_DEF_PARAM(bat_param.para_x, "variable x",
                int,
                "rw", "text: voltage proportion x")
        PFSV1_DEF_PARAM(bat_param.para_y, "variable y",
                int,
                "rw", "text: voltage proportion x")
        PFSV1_DEF_PARAM(bat_param.para_z, "variable z",
                int,
                "rw","text: voltage value offset z")
        PFSV1_DEF_PARAM(bat_param.max_vol, "max voltage",
                int,
                "rw", "text: Battery max voltage (mV)")
        PFSV1_DEF_PARAM(bat_param.max_vol, "min voltage",
                int,
                "rw", "text: Battery min voltage (mV)")
        PFSV1_DEF_PARAM(bat_param.usb_max_vol, "USB charge max voltage",
                int,
                "rw", "text: Battery max voltage when USB charge (mV)")
        PFSV1_DEF_PARAM(bat_param.usb_min_vol, "USB charge min voltage",
                int,
                "rw", "text: Battery min voltage when USB charge (mV)")
        PFSV1_DEF_PARAM(bat_param.ac_max_vol, "AC charge max voltage",
                int,
                "rw", "text: Battery max voltage when AC charge (mV)")
        PFSV1_DEF_PARAM(bat_param.ac_min_vol, "AC charge min voltage",
                int,
                "rw", "text: Battery min voltage when AC charge (mV)")
        PFSV1_DEF_PARAM(bat_param.ac_chg_current, "AC charge current",
                int,
                "ro", "text: AC charge current, determined by the hardware (mA)")
        PFSV1_DEF_PARAM(bat_param.usb_chg_current, "USB charge current",
                int,
                "ro", "text: USB charge current, determined by the hardware (mA)")
    PFSV1_DEF_PARAM_BLOCK_END(bat_param)
PFSV1_DEF_PARAM_END(Battery)

PFSV1_DEF_HELP_ZH_BEGIN(Battery)
	PFSV1_DEF_HELP(bat_param,
		"电池电压由芯片内部ADC转换后获得value, 由于芯片输入最高电压为1.2V, "
		"所以需要在ADC输入端分压, 代码使用x, y作为分压参数, z作为计算偏移量, "
		"计算公式为: batery voltage = (value * x) / y + z)")
	PFSV1_DEF_HELP(bat_param.para_x,
		"分压系数x, 配合参数y, z计算电池电压")
	PFSV1_DEF_HELP(bat_param.para_y,
		"分压系数y, 配合参数x, z计算电池电压")
	PFSV1_DEF_HELP(bat_param.para_z,
		"电压偏移量z, 配合参数x, y计算电池电压")
PFSV1_DEF_HELP_END(Battery)

