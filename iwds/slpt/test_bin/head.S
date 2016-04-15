/*
 * Entry point of the firmware.
 * The firmware code are executed in the ICache.
 */

#include "regdef.h"

	.set noreorder
	.globl _start
	.extern slpt_main
	.text

	.org 0

_start:
    /* Clear BSS */
    la      t1, __bss_start     # t1 <-- __bss_start
    la      t2, __bss_end       # t2 <-- __bss_end
1:
    sw      zero, 0(t1)
    blt     t1, t2, 1b
    add     t1, 4

	/* setup stack, jump to C code */
#	la		$29, 0x10000000		// sp

#	mfc0	$8, $12
#	and		$8, 0xffbff009
#	or		$8, 0x0409
#	mtc0	$8, $12

#	mfc0	$8, $13
#	and		$8, 0xff7fffff
#	or		$8, 0x0
#	mtc0	$8, $13

#	mfc0	$8, $15,1
#	and		$8, 0xfff
#	or		$8, 0x0000000
#	mtc0	$8, $15,1

	j		slpt_init_f
	nop

	.org 0x280
	.word _load_addr
	.word slpt_main
	.word _start
	.word slpt_exit

	.set reorder

