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

#ifndef _JZ47XX_RTC_H
#define _JZ47XX_RTC_H

/* Set the alarm time base on current time */
extern void jz47xx_rtc_set_alarm_period(int);
/* enable/disenable rtc irq */
extern void jz47xx_rtc_enable_irq(void);
extern void jz47xx_rtc_disenable_irq(void);
/* printf infomation debug */
extern void jz47xx_rtc_info(void);
/* get seconds time*/
extern unsigned int jz47xx_rtc_get_realtime(void);
/* get alarm time*/
extern unsigned int jz47xx_rtc_get_alarmtime(void);
/* set seconds time*/
extern void jz47xx_rtc_set_alarmtime(unsigned int time);

extern int jz47xx_rtc_is_enable_irq(void);

#endif
