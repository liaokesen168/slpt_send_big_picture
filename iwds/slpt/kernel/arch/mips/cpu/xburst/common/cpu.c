/*
 * Copyright (C) 2013 Ingenic Semiconductor Co., Ltd.
 * Authors: Kage Shen <kkshen@ingenic.cn>
 *          Duke Fong <duke@dukelec.com>
 *
 * (C) Copyright 2011
 * Xiangfu Liu <xiangfu@openmobilefree.net>
 *
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 3 of the License, or (at your option) any later version.
 */

#include <common.h>
#include <command.h>
#include <netdev.h>
#include <serial.h>
#include <linux/compat.h>
#include <asm/mipsregs.h>
#include <asm/cacheops.h>
#include <asm/reboot.h>
#include <asm/io.h>
#include "timer.h"

#include <slpt_app.h>

#undef	TRACE_DEBUG

#ifdef	TRACE_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

static int __uboot_text_address(unsigned long addr)
{
	if (addr >= (unsigned long)CONFIG_SYS_MONITOR_BASE &&
	    addr < (unsigned long)text_copy_end())
		return 1;
	return 0;
}

static int __uboot_stack_address(unsigned long addr)
{
	if ((addr < CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_INIT_SP_OFFSET)
			&& addr > CONFIG_SYS_SDRAM_BASE)
		return 1;
	return 0;
}

static unsigned long unwind_stack(unsigned long *sp,
		unsigned long pc, unsigned long *ra)
{
	const char *csym;
	unsigned long caddr, size, *i;
	signed long sp_offset, ra_offset;

	PRINTF("unwind: *sp=0x%lx, pc=0x%lx, *ra=0x%lx\n", *sp, pc, *ra);

	if (!__uboot_stack_address(*sp)) {
		PRINTF("unwind: *sp out of range\n");
		return 0;
	}
	if (pc & 0x3 || !__uboot_text_address(pc)) {
		PRINTF("unwind: pc out of range\n");
		return *ra;
	}

	csym = kallsyms_lookup(pc, &caddr, &size);
	if (!csym || caddr & size & 0x3) {
		PRINTF("unwind: csym out of range\n");
		return *ra;
	}

	sp_offset = ra_offset = 0;
	/* TODO: take care with __builtin_alloca */
	for (i = (unsigned long *)pc; i >= (unsigned long *)caddr; i--) {
		PRINTF("unwind: *(%p)=0x%08lx\n", i, *i);
		/* addiu sp,sp,-N */
		if ((*i >> 16) == 0x27bd)
			sp_offset = (s16)(*i & 0xffff);
		/* sw ra,N(sp) */
		if ((*i >> 16) == 0xafbf)
			ra_offset = (s16)(*i & 0xffff);
	}
	PRINTF("unwind: sp_off=0x%lx(-%ld), ra_off=%ld\n",
			sp_offset, -sp_offset, ra_offset);
	if (!sp_offset) {
		PRINTF("unwind: no sp found\n");
		pc = *ra;
		goto end;
	}
	if (!ra_offset) { /* leaf function */
		PRINTF("unwind: leaf\n");
		pc = *ra;
		goto end;
	}

	if (!__uboot_stack_address(*sp + ra_offset)) {
		PRINTF("unwind: can't extract ra from 0x%lx\n",
				*sp + ra_offset);
		return 0;
	}
	pc = *(unsigned long *)(*sp + ra_offset);

end:
	*ra = 0;
	*sp = *sp + (-sp_offset);
	return pc;
}

static void show_raw_backtrace(const struct pt_regs *regs)
{
	unsigned long sp = regs->regs[29];
	unsigned long ra = regs->regs[31];
	unsigned long pc = regs->cp0_epc;

	printk("Call Trace:\n");
	do {
		printk("[<%p>] %pS\n", (void *) pc, (void *) pc);
		pc = unwind_stack(&sp, pc, &ra);
	} while (pc);

	printk("\n");
}

static void show_stacktrace(const struct pt_regs *regs)
{
	const int field = 2 * sizeof(unsigned long);
	int i;
	unsigned long *sp = (unsigned long *)regs->regs[29];

	printk("Stack :");
	if ((unsigned long)sp & 3) {
		printk(" Not aligned!\n");
		return;
	}

	i = 0;
	while ((unsigned long) sp <=
			CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_INIT_SP_OFFSET) {
		if (i && ((i % (64 / field)) == 0))
			printk("\n       ");
		if (i > 39) {
			printk(" ...");
			break;
		}

		printk(" %0*lx", field, *sp++);
		i++;
	}
	printk("\n");
	show_raw_backtrace(regs);
}

static void show_code(unsigned int *pc)
{
	long i;

	printk("\nCode:");

	if ((unsigned long)pc & 3) {
		printk(" Not aligned!\n");
		return;
	}
	for(i = -3 ; i < 6 ; i++) {
		if ((unsigned int)(pc + i) < CKSEG0)
			continue;
		printk("%c%08x%c", (i?' ':'<'), *(pc + i), (i?' ':'>'));
	}
	printk("\n\n");
}

static void __show_cause(unsigned int cause)
{
        unsigned int exc_code = (cause & CAUSEF_EXCCODE) >> CAUSEB_EXCCODE;

	printk("Cause : %08x\n", cause);
	printk("This is ");
        switch (exc_code) {
        case 0:
                printk("Interrupt\n");
                break;
        case 1:
                printk("TLB modification exception\n");
                break;
        case 2:
                printk("TLB exception (load or instruction fetch)\n");
                break;
        case 3:
                printk("TLB exception (store)\n");
                break;
        case 4:
                printk("Address error exception (load or instruction fetch)\n");
                break;
        case 5:
                printk("Address error exception (store)\n");
                break;
        case 6:
                printk("Bus error exception (instruction fetch)\n");
                break;
        case 7:
                printk("Bus error exception (data reference: load or store)\n");
                break;
        case 8:
                printk("Syscall exception\n");
                break;
        case 9:
                printk("Breakpoint exception\n");
                break;
        case 10:
                printk("Reserved instruction exception\n");
                break;
        case 11:
                printk("Coprocessor Unusable exception\n");
                break;
        case 12:
                printk("Arithmetic Overflow exception\n");
                break;
        case 13:
                printk("Trap exception\n");
                break;
        case 16:
                printk("Implementation-Specific Exception 1\n");
                break;
        case 17:
                printk("Implementation-Specific Exception 2\n");
                break;
        case 18:
                printk("Coprocessor 2 exceptions\n");
                break;
        case 23:
                printk("Reference to WatchHi/WatchLo address\n");
                break;
        case 24:
                printk("Machine check\n");
                break;
        default:
                printk("Reserved Bit\n");
                break;
        }
}

static void __show_regs(const struct pt_regs *regs)
{
	const int field = 2 * sizeof(unsigned long);
	unsigned int cause = regs->cp0_cause;

	printk("Cpu 0\n");

	/*
	 * Saved main processor registers
	 */
	printk("zero[$0]=%08lx    at[$1]=%08lx   v0[$2]=%08lx   v1[$3]=%08lx\n",
			regs->regs[0], regs->regs[1], regs->regs[2], regs->regs[3]);
        printk("  a0[$4]=%08lx    a1[$5]=%08lx   a2[$6]=%08lx   a3[$7]=%08lx\n",
                        regs->regs[4], regs->regs[5], regs->regs[6], regs->regs[7]);
	printk("  t0[$8]=%08lx    t1[$9]=%08lx  t2[$10]=%08lx  t3[$11]=%08lx\n",
	                regs->regs[8], regs->regs[9], regs->regs[10], regs->regs[11]);
        printk("  t4[$12]=%08lx  t5[$13]=%08lx  t6[$14]=%08lx  t7[$15]=%08lx\n",
                        regs->regs[12], regs->regs[13], regs->regs[14], regs->regs[15]);
        printk("  s0[$16]=%08lx  s1[$17]=%08lx  s2[$18]=%08lx  s3[$19]=%08lx\n",
                        regs->regs[16], regs->regs[17], regs->regs[18], regs->regs[19]);
        printk("  s4[$20]=%08lx  s5[$21]=%08lx  s6[$22]=%08lx  s7[$23]=%08lx\n",
                        regs->regs[20], regs->regs[21], regs->regs[22], regs->regs[23]);
        printk("  t8[$24]=%08lx  t9[$25]=%08lx  k0[$26]=%08lx  k1[$27]=%08lx\n",
                        regs->regs[24], regs->regs[25], regs->regs[26], regs->regs[27]);
        printk("  gp[$28]=%08lx  sp[$29]=%08lx  s8[$30]=%08lx  ra[$31]=%08lx\n",
                        regs->regs[28], regs->regs[29], regs->regs[30], regs->regs[31]);

	printk("Hi    : %0*lx\n", field, regs->hi);
	printk("Lo    : %0*lx\n", field, regs->lo);

	/*
	 * Saved cp0 registers
	 */
	printk("epc   : %0*lx %pS\n", field, regs->cp0_epc,
	       (void *) regs->cp0_epc);
	printk("ra    : %0*lx %pS\n", field, regs->regs[31],
	       (void *) regs->regs[31]);

	printk("Status: %08x    ", (uint32_t) regs->cp0_status);

	{
		if (regs->cp0_status & ST0_KX)
			printk("KX ");
		if (regs->cp0_status & ST0_SX)
			printk("SX ");
		if (regs->cp0_status & ST0_UX)
			printk("UX ");
		switch (regs->cp0_status & ST0_KSU) {
		case KSU_USER:
			printk("USER ");
			break;
		case KSU_SUPERVISOR:
			printk("SUPERVISOR ");
			break;
		case KSU_KERNEL:
			printk("KERNEL ");
			break;
		default:
			printk("BAD_MODE ");
			break;
		}
		if (regs->cp0_status & ST0_ERL)
			printk("ERL ");
		if (regs->cp0_status & ST0_EXL)
			printk("EXL ");
		if (regs->cp0_status & ST0_IE)
			printk("IE ");
	}
	printk("\n");

        __show_cause(cause);

	cause = (cause & CAUSEF_EXCCODE) >> CAUSEB_EXCCODE;
	if (1 <= cause && cause <= 5)
		printk("BadVA : %0*lx\n", field, regs->cp0_badvaddr);

	printk("PrId  : %08x\n", read_c0_prid());
}

struct pt_regs regs;

void show_registers(struct pt_regs *regs)
{
	struct serial_device *dev = default_serial_console();
	if (dev)
		dev->start();

	printf("Uboot panic\n");
	__show_regs(regs);
	show_stacktrace(regs);
	show_code((unsigned int *) regs->cp0_epc);
#ifndef CONFIG_JZ47XX_SLPT
	hang();
#else
	do_gokernel();
#endif
}

#define cache_op(op, addr)      \
	__asm__ __volatile__(       \
		".set	push\n"         \
		".set	noreorder\n"    \
		".set	mips3\n"        \
		"cache	%0, %1\n"       \
		".set	pop\n"          \
		:                       \
		: "i" (op), "R" (*(unsigned char *)(addr)))

void __attribute__((weak)) _machine_restart(void)
{

	printf("machine_restart\n");

	REG_WDT_TCSR = WDT_TCSR_PRESCALE4 | WDT_TCSR_EXT_EN;
	REG_WDT_TCNT = 0;
	REG_WDT_TDR = 48000000/1000;   	/* reset after 4ms */
	REG_TCU_TSCR = 1 << 16;			/* enable wdt clock */
	REG_WDT_TCER = WDT_TCER_TCEN;  	/* wdt start */

	while (1);

}

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	_machine_restart();

	fprintf(stderr, "*** reset failed ***\n");
	return 0;
}

void flush_cache(ulong start_addr, ulong size)
{
	unsigned long lsize = CONFIG_SYS_CACHELINE_SIZE;
	unsigned long addr = start_addr & ~(lsize - 1);
	unsigned long aend = (start_addr + size - 1) & ~(lsize - 1);

	for (; addr <= aend; addr += lsize) {
		cache_op(HIT_WRITEBACK_INV_D, addr);
		cache_op(HIT_INVALIDATE_I, addr);
	}

	asm volatile ("sync");
}

void flush_dcache_range(ulong start_addr, ulong stop)
{
	unsigned long lsize = CONFIG_SYS_CACHELINE_SIZE;
	unsigned long addr = start_addr & ~(lsize - 1);
	unsigned long aend = (stop - 1) & ~(lsize - 1);

	for (; addr <= aend; addr += lsize)
		cache_op(HIT_WRITEBACK_INV_D, addr);

	asm volatile ("sync");
}

void invalidate_dcache_range(ulong start_addr, ulong stop)
{
	unsigned long lsize = CONFIG_SYS_CACHELINE_SIZE;
	unsigned long addr = start_addr & ~(lsize - 1);
	unsigned long aend = (stop - 1) & ~(lsize - 1);

	for (; addr <= aend; addr += lsize)
		cache_op(HIT_INVALIDATE_D, addr);

	asm volatile ("sync");
}

#define K0BASE			0x80000000

#define Index_Invalidate_I      0x00
#define Index_Writeback_Inv_D   0x01
#define Index_Invalidate_SI     0x02
#define Index_Writeback_Inv_SD  0x03
#define Index_Load_Tag_I	0x04
#define Index_Load_Tag_D	0x05
#define Index_Load_Tag_SI	0x06
#define Index_Load_Tag_SD	0x07
#define Index_Store_Tag_I	0x08
#define Index_Store_Tag_D	0x09
#define Index_Store_Tag_SI	0x0A
#define Index_Store_Tag_SD	0x0B
#define Create_Dirty_Excl_D	0x0d
#define Create_Dirty_Excl_SD	0x0f
#define Hit_Invalidate_I	0x10
#define Hit_Invalidate_D	0x11
#define Hit_Invalidate_SI	0x12
#define Hit_Invalidate_SD	0x13


void flush_icache_all(void)
{
	unsigned int addr, t = 0;

	asm volatile ("mtc0 $0, $28"); /* Clear Taglo */
	asm volatile ("mtc0 $0, $29"); /* Clear TagHi */

	for (addr = K0BASE; addr < K0BASE + CONFIG_SYS_ICACHE_SIZE;
	     addr += CONFIG_SYS_CACHELINE_SIZE) {
		asm volatile (
			".set mips3\n\t"
			" cache %0, 0(%1)\n\t"
			".set mips2\n\t"
			:
			: "I" (Index_Store_Tag_I), "r"(addr));
	}

	/* invalicate btb */
	asm volatile (
		".set mips32\n\t"
		"mfc0 %0, $16, 7\n\t"
		"nop\n\t"
		"ori %0,2\n\t"
		"mtc0 %0, $16, 7\n\t"
		".set mips2\n\t"
		:
		: "r" (t));
}

void flush_dcache_all(void)
{
	unsigned int addr;

	for (addr = K0BASE; addr < K0BASE + CONFIG_SYS_DCACHE_SIZE;
	     addr += CONFIG_SYS_CACHELINE_SIZE) {
		asm volatile (
			".set mips3\n\t"
			" cache %0, 0(%1)\n\t"
			".set mips2\n\t"
			:
			: "I" (Index_Writeback_Inv_D), "r"(addr));
	}

	asm volatile ("sync");
}

void flush_cache_all(void)
{
	flush_dcache_all();
	flush_icache_all();
}

unsigned int get_cp0_reg(void)
{
	unsigned int tmp;
	__asm__ __volatile__ (
		".set mips32        \n\t"
		".set push          \n\t"
		".set noreorder     \n\t"
		".set noat          \n\t"
		"mfc0 %0, $15, 1    \n\t"
		"nop                \n\t"
		".set pop           \n\t"
		:"=r"(tmp)
		:);
	return tmp;
}
