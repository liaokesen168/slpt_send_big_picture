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

#ifndef __SLPT_APP_ALARM_H__
#define __SLPT_APP_ALARM_H__

#include <linux/list.h>

struct alarm_task;

typedef	void (*alarm_task_handler_t)(struct alarm_task *task);

struct alarm_task {
	volatile unsigned int schedulable;
	unsigned int after_run_time;
	struct list_head list;
	alarm_task_handler_t handler;
};

extern struct alarm_task *register_alarm_task(alarm_task_handler_t task_handler);
extern int update_alarm_task(struct alarm_task *task, unsigned int period);

extern int enable_alarm_task(struct alarm_task *task);
extern int disenable_alarm_task(struct alarm_task *task);

#endif
