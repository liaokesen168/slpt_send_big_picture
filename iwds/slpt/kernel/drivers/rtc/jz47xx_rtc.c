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

#include <asm/io.h>

#include <command.h>
#include <rtc.h>

#include <asm/arch/jz47xx_rtc.h>

/*************************************************************************
 * RTC
 *************************************************************************/
#define	RTC_BASE	0xB0003000

#define RTC_RCR		(RTC_BASE + 0x00) /* RTC Control Register */
#define RTC_RSR		(RTC_BASE + 0x04) /* RTC Second Register */
#define RTC_RSAR	(RTC_BASE + 0x08) /* RTC Second Alarm Register */
#define RTC_RGR		(RTC_BASE + 0x0c) /* RTC Regulator Register */

#define RTC_HCR		(RTC_BASE + 0x20) /* Hibernate Control Register */
#define RTC_HWFCR	(RTC_BASE + 0x24) /* Hibernate Wakeup Filter Counter Reg */
#define RTC_HRCR	(RTC_BASE + 0x28) /* Hibernate Reset Counter Register */
#define RTC_HWCR	(RTC_BASE + 0x2c) /* Hibernate Wakeup Control Register */
#define RTC_HWRSR	(RTC_BASE + 0x30) /* Hibernate Wakeup Status Register */
#define RTC_HSPR	(RTC_BASE + 0x34) /* Hibernate Scratch Pattern Register */
#define RTC_WENR	(RTC_BASE + 0x3c) /* Write enable pattern register */
#define RTC_PWRONCR	(RTC_BASE + 0x48) /* Write enable pattern register */

#define REG_RTC_RCR	REG32(RTC_RCR)
#define REG_RTC_RSR	REG32(RTC_RSR)
#define REG_RTC_RSAR	REG32(RTC_RSAR)
#define REG_RTC_RGR	REG32(RTC_RGR)
#define REG_RTC_HCR	REG32(RTC_HCR)
#define REG_RTC_HWFCR	REG32(RTC_HWFCR)
#define REG_RTC_HRCR	REG32(RTC_HRCR)
#define REG_RTC_HWCR	REG32(RTC_HWCR)
#define REG_RTC_HWRSR	REG32(RTC_HWRSR)
#define REG_RTC_HSPR	REG32(RTC_HSPR)
#define REG_RTC_WENR	REG32(RTC_WENR)
#define REG_RTC_PWRONCR	REG32(RTC_PWRONCR)

/* RTC Control Register */
#define RTC_RCR_WRDY	(1 << 7)  /* Write Ready Flag */
#define RTC_RCR_HZ	(1 << 6)  /* 1Hz Flag */
#define RTC_RCR_HZIE	(1 << 5)  /* 1Hz Interrupt Enable */
#define RTC_RCR_AF	(1 << 4)  /* Alarm Flag */
#define RTC_RCR_AIE	(1 << 3)  /* Alarm Interrupt Enable */
#define RTC_RCR_AE	(1 << 2)  /* Alarm Enable */
#define RTC_RCR_RTCE	(1 << 0)  /* RTC Enable */

/* RTC Regulator Register */
#define RTC_RGR_LOCK		(1 << 31) /* Lock Bit */
#define RTC_RGR_ADJC_BIT	16
#define RTC_RGR_ADJC_MASK	(0x3ff << RTC_RGR_ADJC_BIT)
#define RTC_RGR_NC1HZ_BIT	0
#define RTC_RGR_NC1HZ_MASK	(0xffff << RTC_RGR_NC1HZ_BIT)

/* Hibernate Control Register */
#define RTC_HCR_PD		(1 << 0)  /* Power Down */

/* Hibernate Wakeup Filter Counter Register */
#define RTC_HWFCR_BIT		5
#define RTC_HWFCR_MASK		(0x7ff << RTC_HWFCR_BIT)

/* Hibernate Reset Counter Register */
#define RTC_HRCR_BIT		5
#define RTC_HRCR_MASK		(0x7f << RTC_HRCR_BIT)

/* Hibernate Wakeup Control Register */
#define RTC_HWCR_EALM		(1 << 0)  /* RTC alarm wakeup enable */

/* Hibernate Wakeup Status Register */
#define RTC_HWRSR_HR		(1 << 5)  /* Hibernate reset */
#define RTC_HWRSR_PPR		(1 << 4)  /* PPR reset */
#define RTC_HWRSR_PIN		(1 << 1)  /* Wakeup pin status bit */
#define RTC_HWRSR_ALM		(1 << 0)  /* RTC alarm status bit */

/* Write enable pattern register */
#define RTC_WENR_WEN		(1 << 31) /* write has been enabled */
#define RTC_WENR_WENPAT_BIT	0
#define RTC_WENR_WENPAT_MASK	(0xffff << RTC_WENR_WENPAT_BIT) /* The write enable pattern. */
#define RTC_WENR_WENPAT_WRITABLE	(0xa55a)

/* Hibernate Scrach Pattern Register */
#define RTC_HSPR_RTCV      0x52544356 /* The value is 'RTCV', means rtc is valid */
#define RTC_HSPR_RTCA      0x52544341 /* The value is 'RTCA', means rtc poweroff alarm setting */
#define RTC_HSPR_RTCP      0x52544345 /* The value is 'RTCP', means rtc have powered,not first*/
#define RTC_HSPR_RTCU      0x52544355 /* The value is 'RTCU', means rtc reboot from uboot*/

static const unsigned char rtc_days_in_month[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static inline unsigned int is_leap_year(unsigned int year)
{
	return (!(year % 4) && (year % 100)) || !(year % 400);
}

#define LEAPS_THRU_END_OF(y) ((y)/4 - (y)/100 + (y)/400)

/*
 * The number of days in the month.
 */
static int rtc_month_days(unsigned int month, unsigned int year)
{
	return rtc_days_in_month[month] + (is_leap_year(year) && month == 1);
}

/* Converts Gregorian date to seconds since 1970-01-01 00:00:00.
 * Assumes input in normal date format, i.e. 1980-12-31 23:59:59
 * => year=1980, mon=12, day=31, hour=23, min=59, sec=59.
 *
 * [For the Julian calendar (which was used in Russia before 1917,
 * Britain & colonies before 1752, anywhere else before 1582,
 * and is still in use by some communities) leave out the
 * -year/100+year/400 terms, and add 10.]
 *
 * This algorithm was first published by Gauss (I think).
 *
 * WARNING: this function will overflow on 2106-02-07 06:28:16 on
 * machines where long is 32-bit! (However, as time_t is signed, we
 * will already get problems at other places on 2038-01-19 03:14:08)
 */
unsigned long
mktime(const unsigned int year0, const unsigned int mon0,
       const unsigned int day, const unsigned int hour,
       const unsigned int min, const unsigned int sec)
{
	unsigned int mon = mon0, year = year0;

	/* 1..12 -> 11,12,1..10 */
	if (0 >= (int) (mon -= 2)) {
		mon += 12;	/* Puts Feb last since it has leap day */
		year -= 1;
	}

	return ((((unsigned long)
		  (year/4 - year/100 + year/400 + 367*mon/12 + day) +
		  year*365 - 719499
	    )*24 + hour /* now have hours */
	  )*60 + min /* now have minutes */
	)*60 + sec; /* finally seconds */
}

/*
 * Convert seconds since 01-01-1970 00:00:00 to Gregorian date.
 */

void rtc_time_to_tm(unsigned long time, struct rtc_time *tm)
{
	unsigned int month, year;
	int days;

	days = time / 86400;
	time -= (unsigned int) days * 86400;

	/* day of the week, 1970-01-01 was a Thursday */
	tm->tm_wday = (days + 4) % 7;

	year = 1970 + days / 365;
	days -= (year - 1970) * 365
		+ LEAPS_THRU_END_OF(year - 1)
		- LEAPS_THRU_END_OF(1970 - 1);
	if (days < 0) {
		year -= 1;
		days += 365 + is_leap_year(year);
	}
	tm->tm_year = year - 1900;
	tm->tm_yday = days + 1;

	for (month = 0; month < 11; month++) {
		int newdays;

		newdays = days - rtc_month_days(month, year);
		if (newdays < 0)
			break;
		days = newdays;
	}
	tm->tm_mon = month + 1;
	tm->tm_mday = days + 1;

	tm->tm_hour = time / 3600;
	time -= tm->tm_hour * 3600;
	tm->tm_min = time / 60;
	tm->tm_sec = time - tm->tm_min * 60;
}

/*
 * Convert Gregorian date to seconds since 01-01-1970 00:00:00.
 */
int rtc_tm_to_time(struct rtc_time *tm, unsigned int *time)
{
	*time = mktime(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec);
	return 0;
}

int rtc_get (struct rtc_time *tm)
{
	unsigned int current_time = jz47xx_rtc_get_realtime();

	rtc_time_to_tm(current_time, tm);

	return 0;
}

static unsigned int rtc_read_reg(unsigned int reg)
{
	unsigned int data;
	do {
		data = readl(reg);
	} while (readl(reg) != data);
	return data;
}

/* Waiting for the RTC register writing finish */
static void wait_write_ready(void)
{
	unsigned int timeout = 1;
	while (!(rtc_read_reg(RTC_RCR) & RTC_RCR_WRDY) && timeout++);
}

/* Waiting for the RTC register writable */
static void wait_writable(void)
{
	unsigned int timeout = 1;
	wait_write_ready();
	writel(RTC_WENR_WENPAT_WRITABLE, RTC_WENR);
	wait_write_ready();
	while (!(rtc_read_reg(RTC_WENR) & RTC_WENR_WEN) && timeout++);
}

/* RTC write reg ops */
static void rtc_write_reg(unsigned int reg, unsigned int data)
{
	wait_writable();
	writel(data, reg);
	wait_write_ready();
}

int rtc_set (struct rtc_time *tm)
{
	unsigned int current_time;

	rtc_tm_to_time(tm, &current_time);

	rtc_write_reg(RTC_RSR, current_time);

	return 0;
}

#if 0
/*
 * return value : 1 : yes. 0 : no
 * judgement alarm wake up
 */
int is_alarm_wakeup(void)
{
	unsigned int rtccr = 0, rtcsr = 0, rtcsar = 0;
	unsigned int hspr = 0;
	int alarm_startup = 0;

	rtccr = rtc_read_reg(RTC_RCR);
	rtcsr = rtc_read_reg(RTC_RSR);
	rtcsar = rtc_read_reg(RTC_RSAR);
	hspr = rtc_read_reg(RTC_HSPR);

	if (rtc_is_valid() && (rtccr & RTC_RCR_AF) && (rtccr & RTC_RCR_AE) && (rtcsar <= rtcsr)) {
		rtc_write_reg(RTC_RCR,rtccr & (~(RTC_RCR_AIE | RTC_RCR_AE | RTC_RCR_AF)));

#if defined(RTC_HSPR_RTCA) && defined(POWEROFF_ALARM_SIGNATURE)
		if (hspr == RTC_HSPR_RTCA) {
			alarm_startup = 1;
		}
#else
		alarm_startup = 1;
#endif
	}

	return alarm_startup;
}
#endif

/*
 * return value : 1 : need alarm wake up.
 * 		  0 : not
 * set alarm wakeup
 */
#if 0
static int set_alarm_wakeup(void)
{
	unsigned int rtccr = 0, rtcsr = 0, rtcsar = 0, ret = 0;

	rtccr = rtc_read_reg(RTC_RCR);
	rtcsr = rtc_read_reg(RTC_RSR);
	rtcsar = rtc_read_reg(RTC_RSAR);

	/* set wake up valid level as low */
	if (rtcsr < rtcsar && rtc_is_valid()) {
		__intc_unmask_irq(32);
		rtc_write_reg(RTC_RCR,rtccr & (~RTC_RCR_AF));
		rtc_write_reg(RTC_RCR,rtccr | RTC_RCR_AE | RTC_RCR_AIE);
		ret = 1;
	} else {
		rtc_clr_reg(RTC_RCR,RTC_RCR_AF | RTC_RCR_AE | RTC_RCR_AIE);
		rtc_write_reg(RTC_RSAR,0);
	}

	return ret;
}
#endif

/**
 * set a periodical alarm
 * @period : alarm period (unit seconds)
 */
void jz47xx_rtc_set_alarm_period(int period)
{
	unsigned int rtcsr, rtcsar;

	rtcsr = rtc_read_reg(RTC_RSR);
	rtcsar = rtcsr + period;

	rtc_write_reg(RTC_RSAR, rtcsar);

	printf("period time set %d s after [%d]\n", period, rtc_read_reg(RTC_RSAR));
}

void jz47xx_rtc_enable_irq(void)
{
	unsigned int rtccr = rtc_read_reg(RTC_RCR);

	rtc_write_reg(RTC_RCR, (rtccr | RTC_RCR_AE | RTC_RCR_AIE) & (~RTC_RCR_AF));
}

int jz47xx_rtc_is_enable_irq(void)
{
	unsigned int rtccr = rtc_read_reg(RTC_RCR);

	return ((rtccr & RTC_RCR_AE) && (rtccr & RTC_RCR_AIE));
}

void jz47xx_rtc_disenable_irq(void)
{
	unsigned int rtccr = rtc_read_reg(RTC_RCR);

	rtc_write_reg(RTC_RSAR,0);
	rtc_write_reg(RTC_RCR,(rtccr & ~(RTC_RCR_AF | RTC_RCR_AE | RTC_RCR_AIE)));
}

void jz47xx_rtc_info(void)
{
	printf("RCR: 0x%x rtcsr %d rtcsar %d diff \n", rtc_read_reg(RTC_RCR),
			rtc_read_reg(RTC_RSR), rtc_read_reg(RTC_RSAR));
}

unsigned int jz47xx_rtc_get_realtime(void)
{
	return rtc_read_reg(RTC_RSR);
}

unsigned int jz47xx_rtc_get_alarmtime(void)
{
	return rtc_read_reg(RTC_RSAR);
}

void jz47xx_rtc_set_alarmtime(unsigned int time)
{
	rtc_write_reg(RTC_RSAR, time);
}

static int cmd_rtcinfo(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[]) {

	if (argc < 2)
		return CMD_RET_USAGE;

	if (strcmp(argv[1], "info") == 0) {

		if (argc != 2)
			return CMD_RET_USAGE;

		jz47xx_rtc_info();

	} else if (strcmp(argv[1], "period") == 0) {

		unsigned int tmp;

		if (argc != 3)
			return CMD_RET_USAGE;

		tmp = simple_strtoul(argv[2], NULL, 10);

		jz47xx_rtc_set_alarm_period(tmp);

	} else if (strcmp(argv[1], "enable") == 0) {

		if (argc != 2)
			return CMD_RET_USAGE;

		printf("enable RTC xx\n");

		jz47xx_rtc_enable_irq();

	} else if (strcmp(argv[1], "disenable") == 0) {

		if (argc != 2)
			return CMD_RET_USAGE;

		jz47xx_rtc_disenable_irq();

		printf("disenable RTC xx\n");
	} else {
		return CMD_RET_USAGE;
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	rtc, 6, 1, cmd_rtcinfo,
	"rtc [xx/xx] command",
	"info -- show RTC alarm time\n"
	"rtc period -- set period time\n"
	"rtc enable -- enable rtc alarm\n"
	"rtc disenable -- disenable rtc alarm"
	);
