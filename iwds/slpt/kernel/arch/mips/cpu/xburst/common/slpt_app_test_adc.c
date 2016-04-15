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
#include <slpt.h>

#include "slpt_app_alarm.h"
#include "slpt_app_timer.h"

#include <rtc.h>
#include <asm/arch/jz47xx_rtc.h>

#include <fb_struct.h>

#include <power/jz4780_battery.h>
#include <asm/arch/adc.h>

#include <slpt_clock.h>

#include <linux/pr_info.h>
#include <asm/addrspace.h>

#include <i2c.h>

#include "fifo_ring.h"

extern unsigned int match_the_voltage(unsigned int voltage);
extern void low_pmu_voltage_detect(int voltage);

#define I2C_PMU_SLAVE_ADDRESS 0x32

#define	VBATDATAH_REG		0x6A
#define	VBATDATAL_REG		0x6B

static int bus_num;

static struct timer_task *sample_task;
static unsigned int sample_period = 60;
static int is_first_time = 0;
static int is_pmu_adc = 1;
static int is_pmu_ok = 0;
static int is_task_update = 0;
static int adc_power_on_one_time = 0;
static char adc_print_debug[4] = "0";

struct fifo_ring *data_fifo = NULL;

static struct slpt_app_res adc_config_res[] = {
	SLPT_RES_INT_DEF("sample-period", sample_period),
	SLPT_RES_INT_DEF("is-pmu-adc", is_pmu_adc),
	SLPT_RES_INT_DEF("adc-power-on-one-time", adc_power_on_one_time),
	SLPT_RES_ARR_DEF("adc_print_debug", adc_print_debug),
};
SLPT_REGISTER_RES_ARR(adc_config, adc_config_res);

static int i2c_master_recv(unsigned int reg_addr, unsigned char *buff, int len) {
	int status = -1;
	status = mutiple_i2c_read(bus_num, I2C_PMU_SLAVE_ADDRESS, reg_addr, 1, buff, len);
	return status;
}

static int __pmu_read_reg(unsigned int reg_addr, unsigned char *buff, int len) {
	return i2c_master_recv(reg_addr, buff, len);
}

int pmu_get_voltage(unsigned int *data) {
	int ret = 0;
	uint8_t data_l = 0, data_h = 0;

	if (!is_pmu_ok) {
		printf ("Pmu is not ok\n");
		return -1;
	}

	ret = __pmu_read_reg(VBATDATAH_REG, &data_h, 1);
	if (ret < 0) {
		printf("PMU:Error in reading the control register\n");
		return ret;
	}

	ret = __pmu_read_reg(VBATDATAL_REG, &data_l, 1);
	if (ret < 0) {
		printf("PMU:Error in reading the control register\n");
		return ret;
	}

	*data = ((data_h & 0xff) << 4) | (data_l & 0x0f);
	/**data = *data * 1000 * 3 * 5 / 2 / 4095;*/
	*data = *data * 5000 / 4095;

	if (!strncmp("1", adc_print_debug, 1)) {
		printf ("pmu voltage is %d\n", *data);
	}

	return 0;
}

static void adc_task_handler(struct timer_task *task)
{
	unsigned int time;
	unsigned int voltage;
	struct time_vol_pair data;
	int ret = 0;

	is_task_update = 0;

	if (!slpt_kernel_get_sample_adc_for_kernel())
		return ;

	time = jz47xx_rtc_get_realtime();
	if (is_pmu_adc){
		ret = pmu_get_voltage(&voltage);
		data.voltage = voltage;
	} else {
		if (!adc_power_on_one_time)
			jz4780_sadc_init();
		voltage = get_battery_voltage() / 1000;
		if (!adc_power_on_one_time)
			jz4780_sadc_power_off();
		data.voltage = match_the_voltage(voltage);
	}

	data.time = time;

	if(!is_first_time) {
		low_pmu_voltage_detect(data.voltage);
	}

	if (data_fifo && !is_first_time && ret == 0) {
		fifo_ring_add(data_fifo, data);
		if (!strncmp("2", adc_print_debug, 1)) {
			printf("%s voltage_before_match: %d\n", __FUNCTION__, voltage);
			printf("%s voltage_matched: %d\n", __FUNCTION__, data.voltage);
		}
	}
	is_first_time = 0;

	is_task_update = 1;
	update_timer_task(sample_task, sample_period);
}

static int slpt_clock_update(void)
{
	if (!slpt_kernel_get_sample_adc_for_kernel())
		return 0;

	is_first_time = 1;

	if (is_pmu_adc && !is_pmu_ok) {
		bus_num  = mutiple_i2c_probe(I2C_PMU_SLAVE_ADDRESS);
		if(bus_num >= 0) {
			is_pmu_ok = 1;
		}
		if (!is_pmu_ok) {
			printf ("pmu i2c init not ok\n");
		}
	}

	if (!is_pmu_adc && adc_power_on_one_time) {
		printf ("update one time adc\n");
		jz4780_sadc_init();
	}

	if (data_fifo)
		data_fifo->data = (void *)KSEG1ADDR((unsigned long)data_fifo->data);

	if (!is_task_update)
		update_timer_task(sample_task, sample_period);

	return 0;
}
SLPT_CORE_INIT_EVERYTIME(slpt_clock_update);

static int slpt_clock_exit(void) {
	if (!is_pmu_adc && adc_power_on_one_time)
		jz4780_sadc_power_off();

	/* if (data_fifo) */
	/* 	data_fifo->data = (void *)KSEG0ADDR((unsigned long)data_fifo->data); */
	return 0;
}
SLPT_CORE_EXIT_EVERYTIME(slpt_clock_exit);

static int slpt_clock_init(void)
{
	sample_task = register_timer_task(adc_task_handler);
	if (IS_ERR(sample_task)) {
		printf("app clock register alarm task error\n");
		return -ENOMEM;
	}

	data_fifo = slpt_kernel_adc_get_fifo_ring();

	update_timer_task(sample_task, sample_period);
	is_task_update = 1;

	return 0;
}
SLPT_APP_INIT_ONETIME(slpt_clock_init);
