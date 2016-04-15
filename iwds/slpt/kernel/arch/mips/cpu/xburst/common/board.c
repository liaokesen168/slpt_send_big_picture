/*
 *  Copyright (C) 2013 Fighter Sun <wanmyqawdr@126.com>
 *  board initialize codes
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General  Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <serial.h>
#include <stdio_dev.h>
#include <version.h>
#include <environment.h>

#include "init_sequence.h"

#include <asm/arch/board_special.h>
#include <asm/mipsregs.h>

#include <slpt.h>
#include <slpt_app.h>

#include "timer.h"

DECLARE_GLOBAL_DATA_PTR;

ulong monitor_flash_len;

/*
 * mips_io_port_base is the begin of the address space to which x86 style
 * I/O ports are mapped.
 */
const unsigned long mips_io_port_base = -1;


/*
 * All attempts to come up with a "common" initialization sequence
 * that works for all boards and architectures failed: some of the
 * requirements are just _too_ different. To get rid of the resulting
 * mess of board dependend #ifdef'ed code we now make the whole
 * initialization sequence configurable to the user.
 *
 * The requirements for any new initalization function is simple: it
 * receives a pointer to the "global data" structure as it's only
 * argument, and returns an integer return code, where 0 means
 * "continue" and != 0 means "fatal error, hang the system".
 */

int jz47xx_board_init_f(ulong api_addr, struct slpt_task *task)
{
	bd_t *bd;
	ulong addr, addr_malloc;

	/* compiler optimization barrier needed for GCC >= 3.4 */
	__asm__ __volatile__("" : : : "memory");

#ifndef CONFIG_SLPT
	/* unmap kuseg area */
	clear_c0_status(ST0_ERL);
	/* init BTB */
	__write_32bit_c0_register($16,7,0x10);
#endif

	/* Reserve memory for U-Boot code, data & bss
	 * round up to next 16 kB limit
	 */

	addr = bss_end();
	addr |= 16 * 1024 - 1;
	addr++;

	/* initialize the slpt task, and kernel api_addr
	 */
	uboot_slpt_task = task;
	slpt_kernel_get_api_val = (void *)api_addr;

	/*
	 * (permanently) allocate a Board Info struct
	 * and a permanent copy of the "global" data
	 */
	gd = (gd_t *)addr;
	memset((void *)gd, 0, sizeof(gd_t));
	if (initcall_run_list(init_sequence_f))
		hang();

	debug("Reserving %ldk for U-Boot: [%08x, %08lx)\n",
			(bss_end() - CONFIG_SYS_MONITOR_BASE) >> 10,
			CONFIG_SYS_MONITOR_BASE, bss_end());
	debug("Reserving %zu Bytes for Global Data: [%08lx, %08lx)\n",
			sizeof(gd_t), addr, addr + sizeof(gd_t));
	addr += sizeof(gd_t);

	bd = (bd_t *)addr;
	memset((void *)bd, 0, sizeof(bd_t));
	gd->bd = bd;
	debug("Reserving %zu Bytes for Board Info: [%08lx, %08lx)\n",
			sizeof(bd_t), addr, addr + sizeof(bd_t));
	addr += sizeof(bd_t);

	 /* Reserve memory for malloc() arena.
	 */
	debug("Reserving %dk for malloc(): [%08lx, %08lx)\n",
			misc_param.malloc_len >> 10, addr,
			addr + misc_param.malloc_len);
	addr_malloc = addr;
	addr += misc_param.malloc_len;

	/* Reserve memory for boot params.
	 */
	bd->bi_boot_params = addr;
	debug("Reserving %dk for boot params(): [%08lx, %08lx)\n",
			CONFIG_SYS_BOOTPARAMS_LEN >> 10,
			addr, addr + CONFIG_SYS_BOOTPARAMS_LEN);
	/* addr += CONFIG_SYS_BOOTPARAMS_LEN; */

	/*
	 * Finally, we set up a new (bigger) stack.
	 *
	 * Leave some safety gap for SP, force alignment on 16 byte boundary
	 * Clear initial stack frame
	 */
#if 0
	addr_sp -= 16;
	addr_sp &= ~0xF;
	s = (ulong *)addr_sp;
	*s-- = 0;
	*s-- = 0;
	addr_sp = (ulong)s;
#endif
	debug("Stack Pointer at: %08x\n",
			CONFIG_SYS_SDRAM_BASE + CONFIG_SYS_INIT_SP_OFFSET);

	/*
	 * Save local variables to board info struct
	 */
	bd->bi_memstart	= CONFIG_SYS_SDRAM_BASE;	/* start of DRAM */
	bd->bi_memsize	= gd->ram_size;		/* size of DRAM in bytes */
	bd->bi_baudrate	= gd->baudrate;		/* Console Baudrate */

	gd->flags |= GD_FLG_RELOC;	/* tell others: relocation done */

	serial_initialize();

	debug("Now running bottom half, api_addr: %lx\n", api_addr);

	gd->reloc_off = 0;

	monitor_flash_len = image_copy_end() - CONFIG_SYS_MONITOR_BASE;
	/* debug("image_copy_end: %08lx, monitor_flash_len: %08lx\n",
			image_copy_end(), monitor_flash_len); */

	mem_malloc_init(addr_malloc, misc_param.malloc_len);

//	run_initcall_level(init_sequence_r_stage0, (gd_t *)gd);
//	run_initcall_level(init_sequence_r_stage1, (gd_t *)gd);
	run_initcall_level(init_sequence_r_stage2, (gd_t *)gd);
	run_initcall_level(init_sequence_r_stage3, (gd_t *)gd);

	/* Initialize from environment */
	load_addr = getenv_ulong("loadaddr", 16, load_addr);
	debug("load_addr: %08lx\n", load_addr);

#ifndef CONFIG_JZ47XX_SLPT
	gd->have_console = 1;
	/* main_loop() can return to retry autoboot, if so just run it again. */
	for (;;)
		main_loop();
#else
	slpt_initcall_onetime();
#endif

#ifdef CONFIG_SLPT_DEBUG
	gd->have_console = 1;
#endif
	return 0;
	/* NOTREACHED - no way out of command loop except booting */
}

#ifdef CONFIG_JZ47XX_SLPT
void slpt_main_loop(ulong api_addr, struct slpt_task *task)
{

	slpt_printf_kernel_mode = 0;
	debug("slpt_main_loop\n");

	/* main_loop() can return to retry autoboot, if so just run it again. */
	for (;;)
	{
		main_loop();
	}
	/* NOTREACHED - no way out of command loop except booting */
}
#endif

int jz47xx_board_exit(ulong api_addr, struct slpt_task *task)
{
#ifdef CONFIG_JZ47XX_SLPT
	slpt_exitcall_onetime();
#endif
	return 0;
}

#if 0
char slpt_test_mem[] = "this just a test str from task\n";

int jz47xx_slpt_init_f(unsigned long api_addr, struct slpt_task *task)
{
	struct slpt_app_res res_t = {
		.name = "test-mem",
		.type = SLPT_RES_MEM,
		.addr = slpt_test_mem,
		.length = sizeof(slpt_test_mem),
	};
	struct slpt_app_res *res;

	slpt_kernel_get_api_val = (void *)api_addr;

	slpt_kernel_printf("SLPT: info: %s is called\n", __FUNCTION__);

	res = slpt_kernel_register_app_res(&res_t, task);
	if (!res) {
		slpt_kernel_printf("SLPT: error: failed to register res mem-test\n");
		return -ENOMEM;
	}

	return 0;
}

int jz47xx_slpt_exit(unsigned long api_addr, struct slpt_task *task)
{
	struct slpt_app_res *res;

	res = slpt_kernel_name_to_app_res("test-mem", task);
	if (!res) {
		pr_err("SLPT: error: no test-mem res\n");
		return -ENOMEM;
	}

	slpt_kernel_unregister_app_res(res, task);

	slpt_kernel_printf("SLPT: info: %s is called\n", __FUNCTION__);
	return 0;
}
#endif

