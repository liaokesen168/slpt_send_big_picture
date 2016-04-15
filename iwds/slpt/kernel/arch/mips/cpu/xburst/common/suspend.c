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
#include <asm/io.h>

#include <asm/mipsregs.h>

#include <linux/err.h>
#if defined(CONFIG_JZ4780) || defined(CONFIG_JZ4775)
#include <asm/arch/jz4775_gpio.h>
#elif defined(CONFIG_M200)
#include <asm/arch/m200_gpio.h>
#endif
#include <asm/arch/cpm.h>

#include <slpt.h>

void uart_puts(const char *s);

#define TCSM_SAVE_SIZE   4096
#define TCSM_BASE        (0xb3422000)

static unsigned int bakeup_tcsm_flag = 0;
static char tcsm_back[TCSM_SAVE_SIZE] __attribute__ ((aligned (32)));

static inline unsigned long exit_entry_addr(void)
{
	extern ulong _exit;
	return (unsigned long) &_exit;
}

static void xudelay(volatile unsigned int us)
{
	volatile unsigned int temp;
	for ( ; us > 0; us--) {
		for (temp = 0; temp < 130; temp++);
	}
}

void xmdelay(volatile unsigned int ms)
{
	for ( ; ms > 0; ms--) {
		xudelay(1000);
	}
}

void do_suspend(int status)
{
	int ret;

#if 0
//	uart_init();
//	uart_puts("slpt: enable gpio irq\n");
//	REG_GPIO_PXINTS(0) = 1 << 30;
//	REG_GPIO_PXMASKC(0) = 1 << 30;
//	REG_GPIO_PXPAT1S(0) = 1 << 30;
//	REG_GPIO_PXPAT0C(0) = 1 << 30;
//	writel(0x20000, 0xb000100c);
//	_irq(0);

//	__asm__ __volatile__ (
//			".set mips32        \n\t"
//			".set push          \n\t"
//			".set noreorder     \n\t"
//			".set noat          \n\t"
//			"mfc0 $8, $12       \n\t"
//			"ori $9, $8, 0x1f   \n\t"
//			"xori $9, 0x1f      \n\t"
//			"mtc0 $9, $12       \n\t"
//			"nop                \n\t"
//			".set pop           \n\t"
//			:
//			:);

//	writel(0x20000, 0xb0001008);
//	REG_GPIO_PXINTC(0) = 1 << 30;
//	REG_GPIO_PXMASKS(0) = 1 << 30;

//	printf_regs();
//	Xshow_stacktrace();
//	_irq(0);
//	xmdelay(500);
#endif

	debug("slpt: before suspend\n");

	if (status == SLPTARG_LOAD_RESETDLL && (bakeup_tcsm_flag == 0)) {
		bakeup_tcsm_flag = 1;
		memcpy(tcsm_back, (void *)TCSM_BASE, TCSM_SAVE_SIZE);
	}

	ret = slpt_kernel_pm_enter(status);
	if (ret)
		debug("pm: pm enter with error: %d\n", ret);

	debug("slpt: after suspend, pls delay > 300 ms\n");
}

static int cmd_suspend(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[]) {

	/* set PA30(powerkey) irq wakeup */
	REG_GPIO_PXINTS(0) = 1 << 30;
	REG_GPIO_PXMASKC(0) = 1 << 30;
	REG_GPIO_PXPAT1S(0) = 1 << 30;
	REG_GPIO_PXPAT0C(0) = 1 << 30;
	writel(0x20000, 0xb000100c);

	do_suspend(SLPTARG_LOAD_RESETDLL);

	/* clean PA30(powerkey) irq pending bit */
	writel(0x20000, 0xb0001008);
	REG_GPIO_PXINTC(0) = 1 << 30;
	REG_GPIO_PXMASKS(0) = 1 << 30;

	debug("cmd_suspend return\n");
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	suspend, 6, 1, cmd_suspend,
	"suspend system",
	"nothing\n"
);

void do_gokernel(void)
{
#ifndef CONFIG_M200
	if (bakeup_tcsm_flag) {
		memcpy((void *)TCSM_BASE, tcsm_back, TCSM_SAVE_SIZE);
		bakeup_tcsm_flag = 0;
	}
#endif

	slpt_printf_kernel_mode = 1;
	void (*run)(unsigned int) = (void (*)(unsigned int))exit_entry_addr();
	run(0);
}

static int cmd_gokernel(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[]) {
	do_gokernel();
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	gokernel, 6, 1, cmd_gokernel,
	"gokernel return linux-kernel",
	"gokernel go go go !!!\n"
);

void do_idle(void) {
	struct jz4775_cpm_regs *cpm = (void*)CPM_IOBASE;
	unsigned int lcr_save = cpm->lcr;
	unsigned int opcr_save = cpm->opcr;

	cpm->lcr = ((cpm->lcr & ~LCR_LPM_MASK) | 0);
	cpm->opcr = ((cpm->opcr & ~(1 << 30)) | 0);

	__asm__ volatile(".set mips32\n\t"
					 "sync\n\t"
					 "wait\n\t"
					 "nop\n\t"
					 "nop\n\t"
					 "nop\n\t"
					 "nop\n\t"
					 ".set mips32");
	cpm->lcr = lcr_save;
	cpm->opcr = opcr_save;

	/* printf ("lcr : %x\n", lcr_save); */
	/* printf ("opcr: %x\n", opcr_save); */

}

static int cmd_idle(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[]) {

	/* set PA30(powerkey) irq wakeup */
	REG_GPIO_PXINTS(0) = 1 << 30;
	REG_GPIO_PXMASKC(0) = 1 << 30;
	REG_GPIO_PXPAT1S(0) = 1 << 30;
	REG_GPIO_PXPAT0C(0) = 1 << 30;
	writel(0x20000, 0xb000100c);

	do_idle();

	/* clean PA30(powerkey) irq pending bit */
	writel(0x20000, 0xb0001008);
	REG_GPIO_PXINTC(0) = 1 << 30;
	REG_GPIO_PXMASKS(0) = 1 << 30;

	return 0;
}

U_BOOT_CMD(
	idle, 6, 1, cmd_idle,
	"enter idle mode",
	"enter idle mode"
);
