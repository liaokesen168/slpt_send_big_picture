/*
 * analog_minute_clock.h
 *
 *  Created on: May 4, 2015
 *      Author: xblin
 */

#ifndef _ANALOG_MINUTE_CLOCK_H_
#define _ANALOG_MINUTE_CLOCK_H_

#include <analog_base_clock.h>

#ifdef CONFIG_SLPT
#include <slpt.h>
#endif

struct analog_minute_clock {
	struct analog_base_clock clock;

	void (*parent_freev)(struct view *view);

};

#ifdef CONFIG_SLPT
static inline struct slpt_app_res *slpt_register_analog_minute_clock(struct analog_minute_clock *minute_clock,
                                                              struct slpt_app_res *parent) {
	return slpt_register_view(&minute_clock->clock.view, parent, NULL, 0);
}
#endif

#endif /* _ANALOG_MINUTE_CLOCK_H_ */
