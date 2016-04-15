OUTPUT_FORMAT("elf32-tradlittlemips", "elf32-tradlittlemips", "elf32-tradlittlemips")
OUTPUT_ARCH(mips)
ENTRY(_start)

SECTIONS
{
	. = 0x8ff00000;

	_load_addr = .;
	.text : { *(.text*) }

	. = ALIGN(4);
	.rodata : { *(.rodata*) }

	. = ALIGN(4);
	.sdata : { *(.sdata*) }

	. = ALIGN(4);
	.data : { *(.data*) *(.scommon*) *(.reginfo*) }

	_gp = ABSOLUTE(.); /* Base of small data */

	.got : { *(.got*) }

	. = ALIGN(4);
	__bss_start = .;
	.sbss : { *(.sbss*) }
	.bss : { *(.bss*) }
	__bss_end = .;
	. = ALIGN (4);
}

