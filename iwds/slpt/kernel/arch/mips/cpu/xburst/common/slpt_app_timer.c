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

#include <asm/io.h>

#include <malloc.h>
#include <linux/err.h>
#include <linux/list.h>

#include <slpt_app.h>
#include <slpt_irq.h>

#include "timer.h"
#include "slpt_app_timer.h"

#define TIMER_MAX_COUNT (30 * 60)

struct app_timer_param {
	unsigned int timer_count;
	unsigned int next_wakeup_time;
	struct list_head timer_task_list;
};

static struct app_timer_param timer_param;

extern void print_tcu2(void);
extern void clear_tcu2(void);

struct timer_task *register_timer_task(timer_task_handler_t task_handler)
{
	struct timer_task *task;

	if (task_handler == NULL)
		return ERR_PTR(-EINVAL);

	task = malloc(sizeof(struct timer_task));
	if (task == NULL) {
		printf("create alerm task malloc memey error\n");
		return ERR_PTR(-ENOMEM);
	}
	memset(task, 0, sizeof(struct timer_task));

	task->handler = task_handler;

	list_add_tail(&(task->list), &timer_param.timer_task_list);

	debug("register timer task ok [%p] [%p]\n", task, task_handler);

	return task;
}

int enable_timer_task(struct timer_task *task)
{
	if (task == NULL)
		return -EINVAL;

	task->schedulable = 1;
	task->set_run_time = 1;
	return 0;
}

int disenable_timer_task(struct timer_task *task)
{
	if (task == NULL)
		return -EINVAL;

	task->schedulable = 0;
	return 0;
}

int update_timer_task(struct timer_task *task, unsigned int period)
{
	if (task == NULL || period == 0)
		return -EINVAL;

	task->period = period;
	/* when update timer task to enable timer task */
	enable_timer_task(task);

	return 0;
}

void update_wakeup_time(void)
{
	struct list_head *pos;
	struct timer_task *task;
	unsigned int wake_time = 0xffffffff;
	unsigned int tcnt_time = 0;
	unsigned int count;
	unsigned int current_time = 0;

	tcnt_time = read_now_time();
	current_time = tcnt_time + timer_param.timer_count;

	list_for_each(pos, &timer_param.timer_task_list) {
		task = list_entry(pos, struct timer_task, list);
		if (task->set_run_time == 1) {
			task->after_run_time = current_time + task->period;
			task->set_run_time = 0;
		}
		if ((wake_time > task->after_run_time)
				&& task->schedulable && task->handler) {
			wake_time = task->after_run_time;
		}
	}

	if(timer_param.next_wakeup_time == wake_time)
		return;

	timer_param.next_wakeup_time = wake_time;
	count = wake_time - timer_param.timer_count;

	if ((wake_time == 0xffffffff) || (count < 0)) {
		disenable_tcu2();
		return;
	}

	if (count > TIMER_MAX_COUNT)
		count = TIMER_MAX_COUNT;

	update_tcu2(count);
}

static void slpt_timer_handler(unsigned int irq, struct irq_desc *desc)
{
	struct list_head *pos;
	struct timer_task *task;

	debug("irq call %s\n", __func__);

	clear_tcu2_fflag();
	timer_param.timer_count = timer_param.next_wakeup_time;

	list_for_each(pos, &timer_param.timer_task_list) {
		task = list_entry(pos, struct timer_task, list);
		if (task->schedulable && task->handler
				&& (task->after_run_time <= timer_param.next_wakeup_time)) {
			task->schedulable = 0;
			task->handler(task);
		}
	}
}

int slpt_timer_init_everytime(void)
{
	struct list_head *pos;
	struct timer_task *task;

	debug("init call everytime: %s\n", __func__);

	init_tcu_power_and_clock(); //init time tcu power and clock
	clear_tcu2();
	init_tcu2();

	timer_param.timer_count = 0;
	timer_param.next_wakeup_time = 0;

	list_for_each(pos, &timer_param.timer_task_list){
		task = list_entry(pos, struct timer_task, list);
		if (task->schedulable && task->handler) {
			task->schedulable = 0;
			task->handler(task);
		}
	}

	return 0;
}
SLPT_APP_INIT_EVERYTIME(slpt_timer_init_everytime);

static int slpt_timer_init_onetime(void)
{
	int ret = 0;

	debug("init call %s\n", __func__);

	INIT_LIST_HEAD(&timer_param.timer_task_list);

	ret = register_irq_handler(IRQ_TCU2, slpt_timer_handler);
	if (ret) {
		printf("register alarm[%d] irq error !\n", IRQ_TCU2);
		return -EINVAL;
	}

	return 0;
}
SLPT_CORE_INIT_ONETIME(slpt_timer_init_onetime);

static int slpt_timer_exit_everytime(void)
{
	debug("exit call %s\n", __func__);

	disenable_tcu2();
	print_tcu2();
	exit_tcu_power_and_clock();

	return 0;
}
SLPT_APP_EXIT_EVERYTIME(slpt_timer_exit_everytime);
