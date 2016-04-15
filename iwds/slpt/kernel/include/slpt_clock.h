#ifndef _SLPT_CLOCK_H_
#define _SLPT_CLOCK_H_

#include <fb_struct.h>
#include <view.h>
#include <picture.h>
#include <rotate_pic.h>
#include <background.h>
#include <digital_clock.h>
#include <analog_clock.h>
#include <analog_week_clock.h>
#include <analog_month_clock.h>
#include <analog_second_clock.h>
#include <analog_minute_clock.h>
#include <analog_hour_clock.h>
#include <date_en_cn_view.h>
#include <week_en_cn_view.h>
#include <year_en_cn_view.h>
#include <time_view.h>
#include <low_battery.h>
#include <charge_picture.h>

enum clock_type {
	CLOCK_TYPE_DIGITAL,
	CLOCK_TYPE_ANALOG,
	CLOCK_TYPE_ANALOG_MIX_DIGITAL,
	CLOCK_TYPE_BITMASK,

	CLOCK_TYPE_NONE,
};

extern void draw_num_64_32(unsigned int x, unsigned int y, unsigned int num);
extern void draw_maoh_64_32(unsigned int x, unsigned int y, unsigned int flag);
extern void draw_nums(unsigned int x, unsigned int y, unsigned int num);
extern void lcd_draw_clear(void);

/* slpt views */
extern void slpt_display_delete_all_view(void);
extern void slpt_display(void);
extern void slpt_display_sync(void);
extern void slpt_display_init(void);
extern void slpt_display_add_view(struct view *v);

/* slpt analog clock */
extern int slpt_init_analog_clock(void);
extern void slpt_restore_analog_clock(void);
extern void slpt_save_and_draw_analog_clock(void);
extern void slpt_sync_analog_clock(void);
extern int slpt_parse_analog_clock(const char *arg);

/* slpt analog week clock */
extern int slpt_init_analog_week_clock(void);

/* slpt analog month clock */
extern int slpt_init_analog_month_clock(void);

/* slpt analog second clock */
extern int slpt_init_analog_second_clock(void);

/* slpt analog minute clock */
extern int slpt_init_analog_minute_clock(void);

/* slpt analog hour clock */
extern int slpt_init_analog_hour_clock(void);

/* slpt digital clock */
extern int slpt_init_digital_clock(void);
extern void slpt_display_digital_clock(void);
extern void slpt_sync_digital_clock(void);
extern int slpt_parse_digital_clock(const char *arg);

/* slpt date-en-view */
extern int slpt_init_date_en_view(void);
/* slpt date-cn-view */
extern int slpt_init_date_cn_view(void);

/* slpt week-en-view */
extern int slpt_init_week_en_view(void);
/* slpt week-cn-view */
extern int slpt_init_week_cn_view(void);
/* slpt year-cn-view */
extern int slpt_init_year_en_view(void);

/* slpt time-view */
extern int slpt_init_time_view(void);

extern int slpt_init_charge_picture(void);

extern int current_clock_type(void);

extern void update_charger_state(void);
extern void clock_display_task(void);
extern void request_clock_display(void);

#endif /* _SLPT_CLOCK_H_ */
