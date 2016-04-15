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
#include <command.h>

#include <malloc.h>
#include <linux/err.h>
#include <linux/list.h>

#include <rtc.h>
#include <asm/arch/jz47xx_rtc.h>

#include <slpt_app.h>
#include <slpt_irq.h>
#include "slpt_app_alarm.h"

#define DEFINE_ALARM_TIME (0xffffffff)

struct app_alarm_param {
	struct list_head alarm_task_list;
	unsigned int next_alarm_time;
	unsigned int kernel_alarm_time;
};

static struct app_alarm_param alarm_param;

struct alarm_task *register_alarm_task(alarm_task_handler_t task_handler)
{
	struct alarm_task *task;

	if (task_handler == NULL)
		return ERR_PTR(-EINVAL);

	task = malloc(sizeof(struct alarm_task));
	if (task == NULL) {
		printf("create alerm task malloc memey error\n");
		return ERR_PTR(-ENOMEM);
	}
	memset(task, 0, sizeof(struct alarm_task));

	task->handler = task_handler;

	task->after_run_time = DEFINE_ALARM_TIME;

	list_add_tail(&(task->list), &alarm_param.alarm_task_list);

	debug("register alarm task ok [%p] [%p]\n", task, task_handler);

	return task;
}

int enable_alarm_task(struct alarm_task *task)
{
	if (task == NULL)
		return -EINVAL;

	task->schedulable = 1;
	return 0;
}

int disenable_alarm_task(struct alarm_task *task)
{
	if (task == NULL)
		return -EINVAL;

	task->schedulable = 0;
	return 0;
}

int update_alarm_task(struct alarm_task *task, unsigned int period)
{
	static unsigned int current_time;

	if (task == NULL || period == 0)
		return -EINVAL;

	current_time = jz47xx_rtc_get_realtime();

	task->after_run_time = current_time + period;

	/* when update alarm task to enable alarm task */
	enable_alarm_task(task);

	return 0;
}

static inline void slpt_alarm_update_irq(void)
{
	struct list_head *pos;
	struct alarm_task *task;

	alarm_param.next_alarm_time = DEFINE_ALARM_TIME;

	list_for_each(pos, &alarm_param.alarm_task_list) {
		task = list_entry(pos, struct alarm_task, list);
		if ((alarm_param.next_alarm_time > task->after_run_time)
				&& task->schedulable && task->handler) {
			alarm_param.next_alarm_time = task->after_run_time;
		}
	}

	if (alarm_param.next_alarm_time > alarm_param.kernel_alarm_time)
		alarm_param.next_alarm_time = alarm_param.kernel_alarm_time;

	if (alarm_param.next_alarm_time != DEFINE_ALARM_TIME) {
		jz47xx_rtc_set_alarmtime(alarm_param.next_alarm_time);
		jz47xx_rtc_enable_irq();
	}
}

static void slpt_alarm_handler(unsigned int irq, struct irq_desc *desc)
{
	struct list_head *pos;
	struct alarm_task *task;
	struct rtc_time tm;
	static unsigned int current_time;

	debug("irq call %s\n", __func__);

	current_time = jz47xx_rtc_get_realtime();

	rtc_get(&tm);

	debug("RDATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
			tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_wday,
			tm.tm_hour, tm.tm_min, tm.tm_sec );

	if (alarm_param.kernel_alarm_time == alarm_param.next_alarm_time) {
		slpt_mode_exit();
	}

	jz47xx_rtc_disenable_irq();

	list_for_each(pos, &alarm_param.alarm_task_list) {
		task = list_entry(pos, struct alarm_task, list);
		if (task->schedulable && task->handler
				&& (task->after_run_time <= current_time)) {
			task->schedulable = 0;
			task->handler(task);
		}
	}

	slpt_alarm_update_irq();
}

void slpt_rtc_check_alarm(void)
{
	struct rtc_time tm;

	rtc_get(&tm);

	debug("CDATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
			tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_wday,
			tm.tm_hour, tm.tm_min, tm.tm_sec );

	if (jz47xx_rtc_is_enable_irq()) {
		alarm_param.kernel_alarm_time = jz47xx_rtc_get_alarmtime();

		debug("kernel return time [%d] realtime [%d]\n",
				alarm_param.kernel_alarm_time, jz47xx_rtc_get_realtime());
		/* TODO: debug */
//		jz47xx_rtc_disenable_irq();
//		alarm_param.kernel_alarm_time = DEFINE_ALARM_TIME;

	} else
		alarm_param.kernel_alarm_time = DEFINE_ALARM_TIME;
}

static int slpt_alarm_update_status(void)
{
	struct list_head *pos;
	struct alarm_task *task;

	list_for_each(pos, &alarm_param.alarm_task_list){
		task = list_entry(pos, struct alarm_task, list);
		if (task->schedulable && task->handler) {
			task->schedulable = 0;
			task->handler(task);
		}
	}

	slpt_alarm_update_irq();

	return 0;
}
SLPT_APP_INIT_EVERYTIME(slpt_alarm_update_status);

static int slpt_alarm_init(void)
{
	int ret = 0;

	debug("init call %s\n", __func__);

	ret = register_irq_handler(IRQ_RTC, slpt_alarm_handler);
	if (ret) {
		printf("register alarm[%d] irq error !\n", IRQ_RTC);
		return -EINVAL;
	}

	INIT_LIST_HEAD(&alarm_param.alarm_task_list);
	alarm_param.next_alarm_time = DEFINE_ALARM_TIME;

	return 0;
}

SLPT_CORE_INIT_ONETIME(slpt_alarm_init);
