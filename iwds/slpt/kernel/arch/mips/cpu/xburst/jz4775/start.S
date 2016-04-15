/*
 *  Startup Code for MIPS32 XBURST CPU-core
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co., Ltd.
 *          Kage Shen <kkshen@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 3 of the License, or (at your option) any later version.
 */
#include <config.h>
#include <version.h>
#include <asm/regdef.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <asm/cacheops.h>
#include <asm/asm.h>
#include <generated/asm-offsets.h>

    .macro  arch_xburst_save_abi
    .set    push
    .set    noreorder
    .set    noat
    la   k0, CONFIG_SYS_SDRAM_BASE + CONFIG_ARCH_SAVE_REG_OFFSET
    sw   zero, 0(k0)
    sw   ra, 4(k0)
    sw   sp, 8(k0)

    sw   a0, 12(k0)
    sw   a1, 16(k0)
    sw   a2, 20(k0)
    sw   a3, 24(k0)

    sw   s0, 28(k0)
    sw   s1, 32(k0)
    sw   s2, 36(k0)
    sw   s3, 40(k0)
    sw   s4, 44(k0)
    sw   s5, 48(k0)
    sw   s6, 52(k0)
    sw   s7, 56(k0)

    sw   jp, 60(k0) /* PIC jump register */
    sw   gp, 64(k0) /* global pointer */
    sw   fp, 68(k0) /* frame pointer */
    sw   s8, 72(k0) /* same like fp! */
    /* backup Ebase */
    mfc0 t0, $15, 1
    sw   t0, 76(k0)
    /* backup SR */
    mfc0 t0, $12
    sw   t0, 80(k0)

    .set   pop
    .endm

    .macro  arch_xburst_restore_abi
    .set    push
    .set    noreorder
    .set    noat
    la   k0, CONFIG_SYS_SDRAM_BASE + CONFIG_ARCH_SAVE_REG_OFFSET
    lw   ra, 4(k0)
    lw   sp, 8(k0)

    lw   a0, 12(k0)
    lw   a1, 16(k0)
    lw   a2, 20(k0)
    lw   a3, 24(k0)

    lw   s0, 28(k0)
    lw   s1, 32(k0)
    lw   s2, 36(k0)
    lw   s3, 40(k0)
    lw   s4, 44(k0)
    lw   s5, 48(k0)
    lw   s6, 52(k0)
    lw   s7, 56(k0)

    lw   jp, 60(k0) /* PIC jump register */
    lw   gp, 64(k0) /* global pointer */
    lw   fp, 68(k0) /* frame pointer */
    lw   s8, 72(k0) /* same like fp! */
    /* restore Ebase */
    lw   t0, 76(k0)
    mtc0 t0, $15, 1
    /* restore SR */
    lw   t0, 80(k0)
    mtc0 t0, $12

    .set   pop
    .endm

    .set noreorder
    .globl _start
    .globl _exit
    .globl _exception_irq
    .text

#ifdef CONFIG_SLPT_DEBUG
    .org 0
_start:
_tlb_exception:
    j _exception_irq
    nop

   .org 0x180
_other_irq:
    j _exception_irq
    nop

    .org 0x200
_addr_exception:
    j _exception_irq
    nop
#else
    .org 0
_start:
_tlb_exception:
    j slpt_exception_entry
    nop

   .org 0x180
_other_irq:
    j slpt_exception_entry
    nop

    .org 0x200
_addr_exception:
    j slpt_exception_entry
    nop
#endif /* CONFIG_SLPT_DEBUG */

_exception_irq:
	.set noat
    la		k1, regs
    sw		$0, PT_R0(k1)
    sw		$1, PT_R1(k1)
    sw		$2, PT_R2(k1)
    sw		$3, PT_R3(k1)
    sw		$4, PT_R4(k1)
    sw		$5, PT_R5(k1)
    sw		$6, PT_R6(k1)
    sw		$7, PT_R7(k1)
    sw		$8, PT_R8(k1)
    sw		$9, PT_R9(k1)
    sw		$10, PT_R10(k1)
    sw		$11, PT_R11(k1)
    sw		$12, PT_R12(k1)
    sw		$13, PT_R13(k1)
    sw		$14, PT_R14(k1)
    sw		$15, PT_R15(k1)
    sw		$16, PT_R16(k1)
    sw		$17, PT_R17(k1)
    sw		$18, PT_R18(k1)
    sw		$19, PT_R19(k1)
    sw		$20, PT_R20(k1)
    sw		$21, PT_R21(k1)
    sw		$22, PT_R22(k1)
    sw		$23, PT_R23(k1)
    sw		$24, PT_R24(k1)
    sw		$25, PT_R25(k1)
    sw		$26, PT_R26(k1)
    sw		$27, PT_R27(k1)
    sw		$28, PT_R28(k1)
    sw		$29, PT_R29(k1)
    sw		$30, PT_R30(k1)
    sw		$31, PT_R31(k1)
    mfc0	t0, CP0_WATCHLO
    sw		t0, PT_LO(k1)
    mfc0	t0, CP0_WATCHHI
    sw		t0, PT_HI(k1)
    mfc0	t0, CP0_EPC
    sw		t0, PT_EPC(k1)
    mfc0	t0, CP0_BADVADDR
    sw		t0, PT_BVADDR(k1)
    mfc0	t0, CP0_STATUS
    sw		t0, PT_STATUS(k1)
    mfc0	t0, CP0_CAUSE
    sw		t0, PT_CAUSE(k1)

    j		show_registers
    move	a0, k1
    .set at

    .org 0x380
    .word CONFIG_SYS_SDRAM_BASE
    .word _entry
    .word _init_onetime
    .word jz47xx_board_exit

_init_onetime:
    /* Clear BSS */
    la      t1, __bss_start # t1 <-- __bss_start
    la      t2, __bss_end   # t2 <-- __bss_end
1:
    sw      zero, 0(t1)
    blt     t1, t2, 1b
    add     t1, 4

    la      t0, jz47xx_board_init_f
    jr      t0

    nop
	la      v0, 0
    jr ra
    nop

_entry:
    /* Save & Set up temporary stack */

    arch_xburst_save_abi

    la       sp, CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_INIT_SP_OFFSET

    la      t0, 0x8fa00000
    mtc0    t0, $15, 1

//    mfc0    t0, $12
//    and     t0, 0xffbff009
//    or      t0, 0x0409
//    mtc0    t0, $12

//    mfc0    $8, $13
//    and     $8, 0xff7fffff
//    or      $8, 0x0
//    mtc0    $8, $13

//    mfc0    $8, $15,1
//    and     $8, 0xfff
//    or      $8, 0x0000000
//    mtc0    $8, $15,1

    /* mask interrupt */
//    mfc0    t0, $12
//    ori     t1, t0, 0x1f
//    xori    t1, 0x1f
//    mtc0    t1, $12
//-----------------------
//    li      t0, 0x0000Fc05
//    mtc0    t0, $12
    /* Interrupt exception vector set to 0x180 */
//    li      t1, 0x00000000
//    mtc0    t1, $13

    la      t0, slpt_main_loop
//  la      t0, jz47xx_board_init_f
    jr      t0
    nop

_exit:
    la      v0, 0
    arch_xburst_restore_abi
    jr ra
    nop
