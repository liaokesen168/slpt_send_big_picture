/*
 *  Copyright (C) 2013 Fighter Sun <wanmyqawdr@126.com>
 *  ld script for SoC-4780
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
 
#if defined(CONFIG_64BIT)
#define PTR_COUNT_SHIFT	3
#else
#define PTR_COUNT_SHIFT	2
#endif

OUTPUT_ARCH(mips)
ENTRY(_start)
SECTIONS
{
	. = 0x00000000;

	. = ALIGN(4);
	.text : {
		*(.text*)
	}

	. = ALIGN(4);
    __text_copy_end = .;

	.rodata : {
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
	}

	. = ALIGN(4);
	.data : {
		*(.data*)
	}

	. = ALIGN(4);
	.sdata : {
		*(.sdata*)
	}

	. = ALIGN(4);
	pfsv1.datatype.offset : {
		KEEP(*(pfsv1.datatype.offset))
	}

	. = ALIGN(4);
	__pfsv1_param_desc_start = .;
	pfsv1.param.desc : {
		KEEP(*(pfsv1.param.desc));
	}
	. = ALIGN(4);
	__pfsv1_param_desc_end = .;

	. = ALIGN(4);
	__pfsv1_help_zh_cn_utf8_start = .;
	pfsv1.help.zh_cn.utf8 : {
		KEEP(*(pfsv1.help.zh_cn.utf8));
	}
	. = ALIGN(4);
	__pfsv1_help_zh_cn_utf8_end = .;

	. = ALIGN(4);
	__pfsv1_help_en_us_utf8_start = .;
	pfsv1.help.en_us.utf8 : {
		KEEP(*(pfsv1.help.en_us.utf8));
	}
	. = ALIGN(4);
	__pfsv1_help_en_us_utf8_end = .;

	. = ALIGN(4);
	/* slpt arch call section */
	__slpt_initcall_onetime_start = .;
	.slpt_archcall_onetime : { KEEP(*(.slpt_archcall_onetime)); }
	/* slpt core call section */
	.slpt_corecall_onetime : { KEEP(*(.slpt_corecall_onetime)); }
	/* slpt app call section */
	.slpt_initcall_onetime : { KEEP(*(.slpt_initcall_onetime)); }
	. = ALIGN(4);
	__slpt_initcall_onetime_end = .;

	/* slpt arch exit call section */
	__slpt_exitcall_onetime_start = .;
	/* slpt app exit call section */
	.slpt_init_exit_onetime : { KEEP(*(.slpt_init_exit_onetime)); }
	/* slpt core exit call section */
	.slpt_core_exit_onetime : { KEEP(*(.slpt_core_exit_onetime)); }
	/* slpt arch call section */
	.slpt_arch_exit_onetime : { KEEP(*(.slpt_arch_exit_onetime)); }
	. = ALIGN(4);
	__slpt_exitcall_onetime_end = .;

	__slpt_initcall_everytime_start = .;
	.slpt_archcall_everytime : { KEEP(*(.slpt_archcall_everytime)); }
	.slpt_corecall_everytime : { KEEP(*(.slpt_corecall_everytime)); }
	.slpt_initcall_everytime : { KEEP(*(.slpt_initcall_everytime)); }
	. = ALIGN(4);
	__slpt_initcall_everytime_end = .;

	__slpt_exitcall_everytime_start = .;
	.slpt_exitcall_everytime : { KEEP(*(.slpt_exitcall_everytime)); }
	.slpt_core_exit_everytime : { KEEP(*(.slpt_core_exit_everytime)); }
	.slpt_arch_exit_everytime : { KEEP(*(.slpt_arch_exit_everytime)); }
	. = ALIGN(4);
	__slpt_exitcall_everytime_end = .;

	. = ALIGN(4);
	__slpt_res_def_list_start = .;
	.slpt_res_def_list : {KEEP(*(.slpt_res_def_list));}
	. = ALIGN(4);
	__slpt_res_def_list_end = .;

	. = ALIGN(4);
	__slpt_method_def_list_start = .;
	.slpt_method_def_list : {KEEP(*(.slpt_method_def_list));}
	. = ALIGN(4);
	__slpt_method_def_list_end = .;

	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	}

	. = ALIGN(4);
	__image_copy_end = .;

	.rel.dyn : {
		__rel_dyn_start = .;
		*(.rel.dyn)
		__rel_dyn_end = .;
	}

	.deadcode : {
		/*
		 * Workaround for a binutils feature (or bug?).
		 *
		 * The GNU ld from binutils puts the dynamic relocation
		 * entries into the .rel.dyn section. Sometimes it
		 * allocates more dynamic relocation entries than it needs
		 * and the unused slots are set to R_MIPS_NONE entries.
		 *
		 * However the size of the .rel.dyn section in the ELF
		 * section header does not cover the unused entries, so
		 * objcopy removes those during stripping.
		 *
		 * Create a small section here to avoid that.
		 */
		LONG(0xffffffff);
	}

	.dynsym : {
		*(.dynsym)
	}

	.bss __rel_dyn_start (OVERLAY) : {
		__bss_start = .;
		*(.sbss.*)
		*(.bss.*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end = .;
	}

	/DISCARD/ : {
		*(.dynbss)
		*(.dynstr)
		*(.dynamic)
		*(.interp)
		*(.hash)
		*(.gnu.*)
		*(.plt)
		*(.rel.plt)
	}
}
