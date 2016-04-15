/*
 * Authors: Kage Shen <kkshen@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 3 of the License, or (at your option) any later version.
 */

#ifndef __CONFIG_Q8_H
#define __CONFIG_Q8_H

#if 0
#define CFG_UART_BASE       UART3_BASE              /* Base of the UART channel */
#define CFG_EXTAL           (48 * 1000000)

#define CFG_CORE_VOL        1200

#define CFG_CPU_SPEED       (1200 * 1000000)        /* apll */
#define CFG_MEM_SPEED       CFG_CPU_SPEED           /* mpll */
#define CFG_DIV             3                       /* for ddr div */

#define CFG_HZ              (CFG_EXTAL/256)         /* Incrementer freq */
#define CFG_PLL1_FRQ        (432 * 1000000)         /* PLL1 clock */

#define CONFIG_SDRAM_DDR3
/* #define DDR3_ENABEL_ODT */

#endif

#define CONFIG_MEM_SIZE           0x40000000      /* 1GB */
#define CONFIG_SYS_MALLOC_LE      (CONFIG_MEM_SIZE / 2)

/*
 * debug release
 */
#define DEBUG

#define GPIO_I2C0_SDA        (32 * 3 + 30)      /* GPD30 */
#define GPIO_I2C0_SCL        (32 * 3 + 31)      /* GPD31 */

#define GPIO_I2C1_SDA        (32 * 4 + 30)      /* GPE30 */
#define GPIO_I2C1_SCL        (32 * 4 + 31)      /* GPE31 */

#define GPIO_I2C3_SDA        (32 * 3 + 10)      /* GPD10 */
#define GPIO_I2C3_SCL        (32 * 3 + 11)      /* GPD11 */

#define GPIO_MMC_VDD         (32 * 5 + 20)   /*GPF20*/
#define GPIO_FVDD_EN         (32 * 0 + 2)    /*GPA2*/

#define CONFIG_MIPS32        /* MIPS32 CPU core */
#define CONFIG_SYS_LITTLE_ENDIAN
#define CONFIG_JZ4780        /* Jz4780 SoC */
#define CONFIG_JZSOC

#define CONFIG_SYS_HZ        1000

/* We don't use lowlevel_init */
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_BOARD_EARLY_INIT_F

/* ENV related config options */
#define CONFIG_ENV_IS_NOWHERE

/* #define CONFIG_ENV_IS_IN_MMC */
#define CONFIG_ENV_SIZE     (4 << 10)

#define CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_FLASH_BASE   0    /* init flash_base as 0 */

/*
 * DMA configuration
 */
#define CONFIG_JZ4780_DMA

/*
 * MMC configuration
 */
#define CONFIG_GENERIC_MMC      1
#define CONFIG_MMC              1
#define CONFIG_JZ4780_MMC       1
#define CONFIG_DOS_PARTITION    1
/* #define CONFIG_MMC_TRACE     1 */

/*
 * panic configuration
 */
#define CONFIG_PANIC_HANG
#define CONFIG_KALLSYMS

/*
 * MTD & NAND configuration
 */
#define CONFIG_NAND_JZ4780
#define CONFIG_SYS_NAND_SELF_INIT
#define CONFIG_SYS_MAX_NAND_DEVICE  1 /* Max number of NAND devices */
#define MTDIDS_DEFAULT "nand0=jz4780-nand"
#define MTDPARTS_DEFAULT "!!!! DO NOT TOUCH !!!!"
/*
 * TODO: should be carefully consider
 *       if deselect CONFIG_SYS_NAND_SELF_INIT
 */
#ifndef CONFIG_SYS_NAND_SELF_INIT
	#define CONFIG_SYS_NAND_BASE    0x02000000
#endif

#define CONFIG_NAND_ECC_BCH

#define CONFIG_SYS_NAND_ONFI_DETECTION

#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS

/* #define CONFIG_BOOTP_MASK	(CONFIG_BOOTP_DEFAUL) */

#define SPECIAL_BOOTARGS  "mem=32M console=tty0 console=ttyS0,57600n8 ubi.mtd=2 rootfstype=ubifs root=ubi0:rootfs rw rootwait"
#define SPECIAL_BOOTCOMMAND  "battery check; mmc rescan; mmc read 0x80f00000 0x1800 0x6000;go 80f00000"

/*
 * Environment setup
 */
#define CONFIG_BOOTDELAY  30

#define CONFIG_ENV_OVERWRITE

#define CONFIG_EXTRA_ENV_SETTINGS \
	"bootdir=/boot\0" \
	"usbtty=cdc_acm\0" \
	"stdout=serial\0" \
	"stdin=usbtty,serial\0" \
	"stderr=serial\0"

#if 0
#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x82000000\0" \
	"console=ttyO2,115200n8\0" \
	"fdt_high=0xffffffff\0" \
	"usbtty=cdc_acm\0" \
	"vram=16M\0" \
	"mmcdev=0\0" \
	"mmcroot=/dev/mmcblk0p2 rw\0" \
	"mmcrootfstype=ext3 rootwait\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"vram=${vram} " \
		"root=${mmcroot} " \
		"rootfstype=${mmcrootfstype}\0" \
	"loadbootscript=fatload mmc ${mmcdev} ${loadaddr} boot.scr\0" \
	"bootscript=echo Running bootscript from mmc${mmcdev} ...; " \
		"source ${loadaddr}\0" \
	"loaduimage=fatload mmc ${mmcdev} ${loadaddr} uImage\0" \
	"mmcboot=echo Booting from mmc${mmcdev} ...; " \
		"run mmcargs; " \
		"bootm ${loadaddr}\0" \

#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; if mmc rescan; then " \
		"if run loadbootscript; then " \
			"run bootscript; " \
		"else " \
			"if run loaduimage; then " \
				"run mmcboot; " \
			"fi; " \
		"fi; " \
	"fi"
#endif


/*
 * Enabled commands
 */
#define CONFIG_CMD_NAND
#define CONFIG_CMD_MTDPARTS
#define CONFIG_CMD_IMI

#define CONFIG_CMD_GPIO

#define CONFIG_CMD_EXT2
#define CONFIG_CMD_EXT4     /* EXT4 Support */
#define CONFIG_CMD_EXT4_WRITE

#define CONFIG_CMD_FAT      /* FAT support */
#define CONFIG_CMD_I2C      /* I2C serial bus support */
#define CONFIG_CMD_MMC      /* MMC support */

#define CONFIG_CMD_BOOTD    /* bootd */
#define CONFIG_CMD_BOOTZ    /* bootz: boot zImage kernel */

#define CONFIG_CMD_CONSOLE  /* coninfo */
#define CONFIG_CMD_ECHO     /* echo arguments */

#define CONFIG_CMD_LOADB    /* loadb */
#define CONFIG_CMD_LOADS    /* loads */
#define CONFIG_CMD_MEMORY   /* md mm nm mw cp cmp crc base loop mtest */
#define CONFIG_CMD_MISC     /* Misc functions like sleep etc*/
#define CONFIG_CMD_RUN      /* run command in env variable */
#define CONFIG_CMD_SAVEENV  /* saveenv */
#define CONFIG_CMD_SETGETDCR    /* DCR support on 4xx */
#define CONFIG_CMD_SOURCE       /* "source" command support */
#define CONFIG_CMD_ECHOON

/* Console configure */
#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING

/*
 * Serial download configuration
 */
#define CONFIG_LOADS_ECHO  1    /* echo on for serial download */

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_MAXARGS 16
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_PROMPT       "ak47@uboot# "
#define CONFIG_SYS_CBSIZE       1024 * 10    /* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE       (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)


#define CONFIG_SYS_BOOTPARAMS_LEN   (128 * 1024)

#define CONFIG_SYS_SDRAM_BASE       0xc0000000  /* Cached addr */
#define CONFIG_SYS_INIT_SP_OFFSET   (CONFIG_MEM_SIZE - 16)  /* sp point */

#define CONFIG_SYS_LOAD_ADDR        0x8f000000
#define CONFIG_SYS_MEMTEST_START    0x8f000000
#define CONFIG_SYS_MEMTEST_END      0x8f200000

#define CONFIG_SYS_TEXT_BASE        0xc0000000
#define CONFIG_SYS_MONITOR_BASE     CONFIG_SYS_TEXT_BASE

/*
 * Cache Configuration
 */
#define CONFIG_SYS_DCACHE_SIZE      32768 
#define CONFIG_SYS_ICACHE_SIZE      32768 
#define CONFIG_SYS_CACHELINE_SIZE   32

/*
 * libs configuration
 */
#define CONFIG_BCH
#define CONFIG_PFSV1
#define CONFIG_PROGRAMMER
/*
 * misc drivers
 */
#define CONFIG_JZ4780_GPIO
#define CONFIG_ACT8600

#define CONFIG_CONSOLE_MUX  1
#define CONFIG_SYS_CONSOLE_IS_IN_ENV    1

/*
 * usb drivers
 */
#define CONFIG_USBD_HS
#define CONFIG_USBD_MANUFACTURER  "Ingenic"
#define CONFIG_JZ4780_USB
#define CONFIG_USB_DEVICE   1
#define CONFIG_USB_TTY      1

#define CONFIG_USB_DMA
#define CONFIG_ROBUST_USB
#define CONFIG_CMD_BURNER

/*
 * i2c drivers
 */
#define CONFIG_SOFT_I2C
#define CONFIG_SYS_I2C_SPEED    100000      /* 100 kHz */
#define I2C_TRISTATE            { }
#define I2C_DELAY               udelay(25)  /* 1/4 I2C clock duration */
#define I2C_ACTIVE              { }
#define CONFIG_SOFT_I2C_GPIO_SCL        GPIO_I2C0_SCL
#define CONFIG_SOFT_I2C_GPIO_SDA        GPIO_I2C0_SDA
#define CONFIG_SOFT_I2C_READ_REPEATED_START

#endif
