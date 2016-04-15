/*
 * Copyright (C) 2013 Ingenic Semiconductor Co., Ltd.
 * Authors: Kage Shen <kkshen@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 3 of the License, or (at your option) any later version.
 */
#include <config.h>
#include <common.h>

#include <asm/io.h>

#include <asm/arch/uart.h>
#include <asm/arch/board_special.h>

#include "timer.h"
#include <slpt.h>
#include <slpt_app.h>

DECLARE_GLOBAL_DATA_PTR;

static struct jz4780_tcu_regs *tcu_regs = (struct jz4780_tcu_regs *)TCU_BASE;

#define TIMER_OVERFLOW_VAL  0xffff
#define TIMER_CHAN_VAL      0
#define TIMER_REGS_OFFS     (TIMER_CHAN_VAL * 0x10)
#define TIMER_CHANNELx_ADDR(n) ((unsigned int)(n) + TIMER_REGS_OFFS)

#define CPM_SPCR0  ((void*)(0xB0000000 + 0x00B8))  //memory power control for tcu
#define CPM_CLKGR0 ((void*)(0xB0000000 + 0x0020))  //Gate TCU's pclk
#define CPM_OPCR   ((void*)(0xB0000000 + 0x0024))  /*change RTC clock = 32.768KHz */

void reset_timer_masked(void)
{
	/* reset time */
	gd->arch.lastinc = readl(TIMER_CHANNELx_ADDR(&tcu_regs->tcnt0)) & TIMER_OVERFLOW_VAL;
	gd->arch.tbl = 0;
}

ulong get_timer_masked(void)
{
	volatile ulong now = readl(TIMER_CHANNELx_ADDR(&tcu_regs->tcnt0)) & TIMER_OVERFLOW_VAL;

	if (gd->arch.lastinc <= now)
		gd->arch.tbl += now - gd->arch.lastinc; /* normal mode */
	else {
		/* we have an overflow ... */
		gd->arch.tbl += TIMER_OVERFLOW_VAL + now - gd->arch.lastinc;
	}

	gd->arch.lastinc = now;

	return ((gd->arch.tbl * 1000) / 2048);  //rtc clock
}

void udelay_masked(unsigned long usec)
{

}

static void do_timer_init(void) {
	static int inited = 0;

	if (inited)
		return;

	inited = 1;

	writel( readl(CPM_OPCR) | 1 << 2, CPM_OPCR); /*change RTC clock = 32.768KHz */

	writeb(1 << TIMER_CHAN_VAL, &tcu_regs->tecr); /* stop counting up */

	/* Internal clock: CLK/16, rtc clk input*/
	writel(TCU_TCSR_PRESCALE16 | TCU_TCSR_RTC_EN,
			TIMER_CHANNELx_ADDR(&tcu_regs->tcsr0));

	/* clean tcnt0 reg */
	writel(0, TIMER_CHANNELx_ADDR(&tcu_regs->tcnt0));
	writel(0, TIMER_CHANNELx_ADDR(&tcu_regs->tdhr0));
	writel(TIMER_OVERFLOW_VAL, TIMER_CHANNELx_ADDR(&tcu_regs->tdfr0));

	/* mask irqs */
	writel((1 << TIMER_CHAN_VAL) | (1 << (TIMER_CHAN_VAL + 16)), &tcu_regs->tmsr);
	writel(1 << TIMER_CHAN_VAL, &tcu_regs->tscr); /* enable timer clock */
	writeb(1 << TIMER_CHAN_VAL, &tcu_regs->tesr); /* start counting up */
}

int timer_init(void)
{
#ifndef CONFIG_SLPT
	do_timer_init();
#endif

	gd->arch.lastinc = 0;
	gd->arch.tbl = 0;

	return 0;
}

static int timer_init_everytime(void) {
	do_timer_init();
	return 0;
}
SLPT_ARCH_INIT_EVERYTIME(timer_init_everytime);

void clear_tcu(unsigned int chanel) {
	writel(((1 << chanel) | (1 << (chanel + 8)) | (1 << (chanel + 16)) | (1 << (chanel + 24))), &tcu_regs->tfcr);
	writel(((1 << chanel) | (1 << (chanel + 8)) | (1 << (chanel + 16)) | (1 << (chanel + 24))), &tcu_regs->tmsr);
}

void reset_timer(void)
{
	reset_timer_masked();
}

/*
 * timer without interrupts
 */
ulong get_timer(ulong base)
{
	ulong tmp = get_timer_masked();

	if(tmp >= base)
		return (ulong)(tmp - base);
	else
		return tmp;
}

void set_timer(ulong t)
{

}

inline void __delay(unsigned int loops)
{
	__asm__ __volatile__ (
		".set noreorder \n"
		".align 3 \n"
		"1:bnez %0, 1b \n"
		"subu %0, 1\n"
		".set reorder \n"
		: "=r" (loops)
		: "0" (loops)
	);
}

/*
 * Division by multiplication: you don't have to worry about
 * loss of precision.
 *
 * Use only for very small delays ( < 1 msec).  Should probably use a
 * lookup table, really, as the multiplications take much too long with
 * short delays.  This is a "reasonable" implementation, though (and the
 * first constant multiplications gets optimized away if the delay is
 * a constant)
 */

void __udelay(unsigned long usec)
{
	unsigned int i = usec * (misc_param.cpu_freq / 2);
	__delay(i);
}

#if 0
void ndelay(unsigned long usec)
{
	u32 i = usec * board_boot_para.sys_para.cpu_speed;

	if(i >= 2000)
		i /= 2000;
	else
		i=1;

	__delay(i);
}
#endif

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On MIPS it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return (unsigned long long)get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On MIPS it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}

#undef TIMER_CHAN_VAL
#define TIMER_CHAN_VAL   2

void print_tcu2(void) {
	debug ("tdfr0 :%d\n", readl(TIMER_CHANNELx_ADDR(&tcu_regs->tdfr0)));
	debug ("tcnt0 :%d\n", readl(TIMER_CHANNELx_ADDR(&tcu_regs->tcnt0)));
}

void clear_tcu2_fflag(void)
{
	writel(1 << TIMER_CHAN_VAL, &tcu_regs->tfcr);
}

void enable_tcu2(void)
{
	writel(1 << TIMER_CHAN_VAL, &tcu_regs->tfcr);
	writeb(1 << TIMER_CHAN_VAL, &tcu_regs->tesr); /* start counting up */
}

void disenable_tcu2(void)
{
	writel(1 << TIMER_CHAN_VAL, &tcu_regs->tfcr);
	writel(1 << TIMER_CHAN_VAL, &tcu_regs->tmsr);
	writeb(1 << TIMER_CHAN_VAL, &tcu_regs->tecr); /* stop counting up */
}

/*
 *	There are two modes of TCU for the eight channels
 *	 TCU1: Channel 0, 3,4, 5, 6,and 7
 *	 TCU2: Channel 1,2
 */
void print_bug(int flag)
{
	if(flag == 0) {
		printf("****************************************************\n");
		printf("****************************************************\n");
		printf("***********   THE TCU2 COUNT OVERFLOW!   ***********\n");
		printf("****************************************************\n");
		printf("****************************************************\n");
	} else {
		printf("****************************************************\n");
		printf("****************************************************\n");
		printf("***   THE IRQ PROCESSING TIME EXCEED 1 SECOND!   ***\n");
		printf("****************************************************\n");
		printf("****************************************************\n");
	}
}

unsigned int read_now_time(void)
{
	unsigned int tcnt = 0;
	unsigned int time = 0;
	unsigned int tfr = readl(&tcu_regs->tfr);

	if(tfr & (1 << TIMER_CHAN_VAL))
		tcnt = readl(TIMER_CHANNELx_ADDR(&tcu_regs->tdfr0));
	else
		tcnt = readl(TIMER_CHANNELx_ADDR(&tcu_regs->tcnt0));
	if(tcnt % 32 >= 30)
		time = tcnt / 32 + 1;
	else
		time = tcnt / 32;

	return time;
}

void update_tcu2(unsigned int time)
{
	unsigned int tcnt;
	unsigned int tm = time;
	unsigned int tfr = readl(&tcu_regs->tfr);

	if(tfr & (1 << TIMER_CHAN_VAL)) {
		tm -= read_now_time();
		tm = tm * 32 -1;
	}
	else
		tm = tm * 32 -1;

	if (tm > 0xffff) {
		print_bug(0);
		return;
	}

	tcnt = readl(TIMER_CHANNELx_ADDR(&tcu_regs->tcnt0));

	if (tcnt > tm) {
		print_bug(1);
		return;
	}

	writel(tm, TIMER_CHANNELx_ADDR(&tcu_regs->tdfr0));
	enable_tcu2();

	return;
}

void clear_tcu2(void)
{
#if 0
	int reset_cnt = 0;
	int reset_cnt2 = 0;

	if (readl(TIMER_CHANNELx_ADDR(&tcu_regs->tcnt0)) != 0) {
		writel(readl(TIMER_CHANNELx_ADDR(&tcu_regs->tcnt0)) + 1, TIMER_CHANNELx_ADDR(&tcu_regs->tdfr0));
		print_tcu2();
		while (readl(TIMER_CHANNELx_ADDR(&tcu_regs->tcnt0)) != 0) {
			reset_cnt++;
			mdelay(1);
			if (reset_cnt >= 50) {
				printf ("reset cnt over run\n");
				break;
			}
		}
	}

	if (readl(TIMER_CHANNELx_ADDR(&tcu_regs->tcnt0)) != 0) {
		writel(readl(TIMER_CHANNELx_ADDR(&tcu_regs->tcnt0)) + 1, TIMER_CHANNELx_ADDR(&tcu_regs->tdfr0));
		print_tcu2();
		while (readl(TIMER_CHANNELx_ADDR(&tcu_regs->tcnt0)) != 0) {
			reset_cnt2++;
			mdelay(1);
			if (reset_cnt2 >= 50) {
				printf ("reset cnt2 over run\n");
				break;
			}
		}
	}

	writel(0, TIMER_CHANNELx_ADDR(&tcu_regs->tcnt0));

	debug ("timer wait reset cnt: %d\n", reset_cnt);
	debug ("timer wait reset cnt2: %d\n", reset_cnt2);

#else
	int reset_cnt3 = 0;

	if (readl(TIMER_CHANNELx_ADDR(&tcu_regs->tcnt0)) != 0) {
		writel(readl(TIMER_CHANNELx_ADDR(&tcu_regs->tcsr0)) | TCU_TCSR_CLRZ,
			   TIMER_CHANNELx_ADDR(&tcu_regs->tcsr0));
		/* while (readl(TIMER_CHANNELx_ADDR(&tcu_regs->tcnt0)) != 0) { */ /* for other tcu */
		while (readl(TCU_TSTR) & (1 << TIMER_CHAN_VAL)) { /* just only tcu1, tcu2 */
			reset_cnt3++;
			mdelay(1);
			if (reset_cnt3 >= 100) {
				printf ("reset cnt3 over run\n");
				break;
			}
		}
	}

	debug ("timer wait reset cnt3: %d\n", reset_cnt3);
#endif
	print_tcu2();

	return;
}

void init_tcu2(void)
{
	/* init TCU2 mode Reset the TCU */
	writel(TCU_TCSR_PRESCALE16 | TCU_TCSR_PCK_EN,
			TIMER_CHANNELx_ADDR(&tcu_regs->tcsr0));
	writel(1 << 10, TIMER_CHANNELx_ADDR(&tcu_regs->tcsr0));
	writel(0, TIMER_CHANNELx_ADDR(&tcu_regs->tcnt0));
	writel(TCU_TCSR_PRESCALE16, TIMER_CHANNELx_ADDR(&tcu_regs->tcsr0));

	/* init TCU2 mode Initial the configuration */
	writel(1 << TIMER_CHAN_VAL, &tcu_regs->tmcr);

	/* Internal clock: CLK/16, rtc clk input*/
	writel(TCU_TCSR_PRESCALE1024 | TCU_TCSR_RTC_EN,
			TIMER_CHANNELx_ADDR(&tcu_regs->tcsr0));

	/* clean tcnt0 reg */
	writel(0, TIMER_CHANNELx_ADDR(&tcu_regs->tcnt0));
	writel(0, TIMER_CHANNELx_ADDR(&tcu_regs->tdhr0));

	clear_tcu2();

	*(unsigned int *)(0xB000100c) = (1 << 25);
}

void init_tcu_power_and_clock(void)
{
#ifdef CONFIG_M200
	writel(readl(CPM_SPCR0) & (~(1<<21)), CPM_SPCR0);  //memory power control for tcu
	writel(readl(CPM_CLKGR0) & (~(1<<30)), CPM_CLKGR0);  //Gate TCU's pclk
#endif
}


void exit_tcu_power_and_clock(void)
{
#ifdef CONFIG_M200
	writel(readl(CPM_SPCR0) | (1<<21), CPM_SPCR0);  //memory power control for tcu
	writel(readl(CPM_CLKGR0) | (1<<30), CPM_CLKGR0);  //Gate TCU's pclk
#endif
}
