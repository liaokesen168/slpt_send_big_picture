#ifndef _CURRENT_TIME_H_
#define _CURRENT_TIME_H_

#include <rtc.h>

struct cur_time {
	unsigned long time;
	struct rtc_time tm;
};

extern struct cur_time cur_time;

static inline unsigned long get_currnet_time(void) {
	return cur_time.time;
}

static inline struct rtc_time *get_currnet_tm(void) {
	return &cur_time.tm;
}

void set_current_time(unsigned long time);

#endif /* _CURRENT_TIME_H_ */
