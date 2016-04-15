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

#ifndef __SLPT_APP_TIMER_H__
#define __SLPT_APP_TIMER_H__

#include <linux/list.h>

struct timer_task;

typedef	void (*timer_task_handler_t)(struct timer_task *task);

struct timer_task {
	volatile unsigned int schedulable;
	volatile unsigned int set_run_time;
	unsigned int after_run_time;
	struct list_head list;
	timer_task_handler_t handler;
	unsigned int period;
};

extern struct timer_task *register_timer_task(timer_task_handler_t task_handler);
extern int update_timer_task(struct timer_task *task, unsigned int period);

extern int enable_timer_task(struct timer_task *task);
extern int disenable_timer_task(struct timer_task *task);

#endif
