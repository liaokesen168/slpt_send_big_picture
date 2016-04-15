/*
 * Authors: Kage Shen <kkshen@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 3 of the License, or (at your option) any later version.
 */

#ifndef __CONFIG_S2122B_H
#define __CONFIG_S2122B_H

#define CONFIG_MEM_SIZE           0x600000      /* 6MB */
#define CONFIG_SYS_MALLOC_LE      0x300000      /* malloc memory size 2MB */

/*
 * debug release
 */
//#define DEBUG
#define PR_ERR_DEBUG 0

#define GPIO_I2C0_SDA        (32 * 3 + 30)      /* GPD30 */
#define GPIO_I2C0_SCL        (32 * 3 + 31)      /* GPD31 */

#define GPIO_I2C1_SDA        (32 * 4 + 30)      /* GPE30, TP, if define it for soft i2c,it will affect the kernel tp */
#define GPIO_I2C1_SCL        (32 * 4 + 31)      /* GPE31, TP */

#define GPIO_I2C2_SDA        (32 * 4 + 0)       /* GPE0 */
#define GPIO_I2C2_SCL        (32 * 4 + 3)       /* GPE3 */

#define GPIO_I2C3_SDA        (32 * 3 + 28)      /* GPD28 */
#define GPIO_I2C3_SCL        (32 * 3 + 29)      /* GPD29 */

#define GPIO_MMC_VDD         (32 * 5 + 20)		/*GPF20*/
#define GPIO_FVDD_EN         (32 * 0 + 2)		/*GPA2*/

#define CONFIG_MIPS32        /* MIPS32 CPU core */
#define CONFIG_SYS_LITTLE_ENDIAN
#define CONFIG_M200        /* M200 SoC */
#define CONFIG_JZSOC

/* compatible with voice trigger, we map to the address of kseg2, and use 64 kb second cache */
#define CONFIG_SLPT_MAP_TO_KSEG2

/* some code move form kernel need this macro */
#define CONFIG_JZRISC

#define CONFIG_SYS_HZ        1000

/* We don't use lowlevel_init */
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_BOARD_EARLY_INIT_F

/* ENV related config options */
#define CONFIG_ENV_IS_NOWHERE

#define CONFIG_ENV_SIZE     (4 << 10)

#define CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_FLASH_BASE   0    /* init flash_base as 0 */

/*
 * DMA configuration
 */
#define CONFIG_M200_DMA
#define CONFIG_KALLSYMS

/*
 * panic configuration
 */
#define CONFIG_PANIC_HANG

/* #define CONFIG_BOOTP_MASK	(CONFIG_BOOTP_DEFAUL) */
#define SPECIAL_BOOTARGS		"mem=256M@0x0 mem=240M@0x30000000 console=ttyS3,57600n8 ip=off root=/dev/ram0 rw rdinit=/init pmem_camera=16M@0x3f000000"

//#define SPECIAL_BOOTARGS  "mem=32M console=tty3 console=ttyS3,57600n8 ubi.mtd=2 rootfstype=ubifs root=ubi0:rootfs rw rootwait"
//#define SPECIAL_BOOTCOMMAND  "battery check; mmc rescan; mmc read 0x80f00000 0x1800 0x6000;go 80f00000"
#define SPECIAL_BOOTCOMMAND  "slpt"

/*
 * Environment setup
 */
#define CONFIG_BOOTDELAY  0

#define CONFIG_ENV_OVERWRITE

#define CONFIG_EXTRA_ENV_SETTINGS \
	"bootdir=/boot\0" \
	"usbtty=cdc_acm\0" \
	"stdout=usbtty,serial\0" \
	"stdin=usbtty,serial\0" \
	"stderr=usbtty,serial\0"



#if 0
/*
 * MMC configuration
 */
#define CONFIG_GENERIC_MMC      1
#define CONFIG_MMC              1
#define CONFIG_M200_MMC       1
#define CONFIG_DOS_PARTITION    1
#define CONFIG_MMC_TRACE

/*
 * MTD & NAND configuration
 */
#define CONFIG_NAND_M200
#define CONFIG_SYS_NAND_SELF_INIT
#define CONFIG_SYS_MAX_NAND_DEVICE  1 /* Max number of NAND devices */
#define MTDIDS_DEFAULT "nand0=m200-nand"
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


/*
 * Enabled commands
 */
#define CONFIG_CMD_NAND
#define CONFIG_CMD_MTDPARTS
#define CONFIG_CMD_IMI

/*
 * libs configuration
 */
#define CONFIG_BCH
#define CONFIG_PFSV1
#define CONFIG_PROGRAMMER
#endif

#define CONFIG_CMD_GPIO

//#define CONFIG_CMD_EXT2
//#define CONFIG_CMD_EXT4     /* EXT4 Support */
//#define CONFIG_CMD_EXT4_WRITE

#define CONFIG_CMD_FAT      /* FAT support */
//#define CONFIG_CMD_I2C      /* I2C serial bus support */
//#define CONFIG_CMD_MMC      /* MMC support */

#define CONFIG_CMD_BOOTD    /* bootd */
#define CONFIG_CMD_BOOTZ    /* bootz: boot zImage kernel */

#define CONFIG_CMD_CONSOLE  /* coninfo */
#define CONFIG_CMD_ECHO     /* echo arguments */

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
#define CONFIG_SYS_PROMPT       "slpt@uboot# "
#define CONFIG_SYS_CBSIZE       1024 * 10             /* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE       (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

#define CONFIG_SYS_BOOTPARAMS_LEN   (16 * 1024)

#ifdef CONFIG_SLPT_MAP_TO_KSEG2
#define CONFIG_SYS_SDRAM_BASE       0xc0000000   /* Cached addr at 0xc0000000 */
#else
#define CONFIG_SYS_SDRAM_BASE       0x8FA00000   /* Cached addr at 250MB */
#endif
#define CONFIG_SYS_INIT_SP_OFFSET   (CONFIG_MEM_SIZE - (16 * 1024))    /* sp point */
#define CONFIG_ARCH_SAVE_REG_OFFSET   (CONFIG_MEM_SIZE - (1 * 1024))

#define CONFIG_SYS_LOAD_ADDR        0x0

#ifdef CONFIG_SLPT_MAP_TO_KSEG2
#define CONFIG_SYS_TEXT_BASE        0xc0000000
#else
#define CONFIG_SYS_TEXT_BASE        0x8FA00000
#endif

#define CONFIG_SYS_MONITOR_BASE     CONFIG_SYS_TEXT_BASE

/*
 * Cache Configuration
 */
#define CONFIG_SYS_DCACHE_SIZE      32768
#define CONFIG_SYS_ICACHE_SIZE      32768
#define CONFIG_SYS_CACHELINE_SIZE   32

#define CONFIG_SYS_ICACHE_WAYSIZE 4096
#define CONFIG_SYS_ICACHE_WAYBIT 12
#define CONFIG_SYS_ICACHE_WAYS 8
#define CONFIG_SYS_ICACHE_LINE_SIZE 32

#define CONFIG_SYS_DCACHE_WAYSIZE 4096
#define CONFIG_SYS_DCACHE_WAYBIT 12
#define CONFIG_SYS_DCACHE_WAYS 8
#define CONFIG_SYS_DCACHE_LINE_SIZE 32

#define CONFIG_SYS_SCACHE_WAYSIZE 65536
#define CONFIG_SYS_SCACHE_WAYBIT 16
#define CONFIG_SYS_SCACHE_WAYS 8
#define CONFIG_SYS_SCACHE_LINE_SIZE 32

/*
 * misc drivers
 */
#define CONFIG_M200_GPIO
//#define CONFIG_ACT8600

#define CONFIG_CONSOLE_MUX  1
#define CONFIG_SYS_CONSOLE_IS_IN_ENV    1

/*
 * usb drivers
 */
//#define CONFIG_USBD_HS
//#define CONFIG_USBD_MANUFACTURER  "Ingenic"
//#define CONFIG_M200_USB
//#define CONFIG_USB_DEVICE   1
//#define CONFIG_USB_TTY      1

//#define CONFIG_USB_DMA
//#define CONFIG_ROBUST_USB
//#define CONFIG_CMD_BURNER

/*
 * rtc drivers
 */

#define CONFIG_JZ47XX_RTC

/*
 * i2c drivers
 */
#define CONFIG_SOFT_I2C
/* #define CONFIG_MUTIPLE_I2C_BUS */			/* mutiple i2c bus enable */

#define CONFIG_SYS_I2C_SPEED    100000      /* 100 kHz */
#define I2C_TRISTATE            { }
#define I2C_DELAY               udelay(25)  /* 1/4 I2C clock duration */
#define I2C_ACTIVE              { }
#define CONFIG_SOFT_I2C_GPIO_SCL        GPIO_I2C0_SCL
#define CONFIG_SOFT_I2C_GPIO_SDA        GPIO_I2C0_SDA

#ifndef GPIO_I2C0_SCL
#define GPIO_I2C0_SCL (32 * 3 + 31)      /* GPD31 */
#endif
#ifndef GPIO_I2C0_SDA
#define GPIO_I2C0_SDA (32 * 3 + 30)      /* GPD30 */
#endif

#ifdef CONFIG_MUTIPLE_I2C_BUS

#ifndef CONFIG_SOFT_I2C_GPIO_SCL
#define CONFIG_SOFT_I2C_GPIO_SCL GPIO_I2C0_SCL
#endif

#ifndef CONFIG_SOFT_I2C_GPIO_SDA
#define CONFIG_SOFT_I2C_GPIO_SDA GPIO_I2C0_SDA
#endif

#define CONFIG_SOFT_I2C_GPIO_SCL0	CONFIG_SOFT_I2C_GPIO_SCL
#define CONFIG_SOFT_I2C_GPIO_SDA0	CONFIG_SOFT_I2C_GPIO_SDA

/*
#define CONFIG_SOFT_I2C_GPIO_SCL1	GPIO_I2C3_SCL
#define CONFIG_SOFT_I2C_GPIO_SDA1	GPIO_I2C3_SDA
*/

#endif

#define CONFIG_SOFT_I2C_READ_REPEATED_START

/*
 * lcd driver
 */
#define CONFIG_JZ_FB
#define CONFIG_JZ_MIPI_DSI 		/* for m200 only */
#define CONFIG_SLCD_SEPS645B
#define CONFIG_SLCD_TRULY240240
#define CONFIG_SLCD_TRULY320320
#define CONFIG_LCD_X163
#define CONFIG_LCD_BOE_TFT320320
#define CONFIG_LCD_AUO_H139BLN01
#define CONFIG_LCD_EDO_E1392AM1
#define CONFIG_LCD_H160_TFT320320
#define CONFIG_LCD_ARS_NT35350

/*
 * slpt configs
 */
#define CONFIG_SLPT

#define TCSM_PCHAR(x)                                       \
	while ((*((volatile unsigned int*)(0xb0033000+0x14)) &  \
	((1 << 5) | (1 << 6))) != ((1 << 5) | (1 << 6)));       \
	*((volatile unsigned int*)(0xb0033000+0)) = x

#endif

#define CONFIG_SLPT_DEBUG
#define CONFIG_JZ47XX_SLPT
#define CONFIG_SLPT_CLOCK
#define CONFIG_SLPT_ALARM
#define CONFIG_SLPT_TIMER
/* #define CONFIG_SLPT_POWERKEY */
/* #define CONFIG_FRIZZ */

/* battery */
#define CONFIG_SLPT_BATTERY
#define BATTERY_WARNING_VOLTAGE 3407
#define BATTERY_LOW_VOLTAGE 3625

#define SLPT_DEBUG_PIN (32 * 1 + 5) // PB5

/* picture not use bmp, use fb region directly */
#define CONFIG_PICTURE_NO_BMP

/* choose view display */
/* #define CONFIG_VIEW */
#define CONFIG_SVIEW

/* double buffering of fb */
#define CONFIG_FB_DOUBLE_BUFFERING

/*
 * test case
 */
/*#define CONFIG_POWER_TEST_LCD*/
