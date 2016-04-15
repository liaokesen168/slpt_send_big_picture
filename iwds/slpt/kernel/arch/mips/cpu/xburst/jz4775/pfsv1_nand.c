/*
 *  Copyright (C) 2013 Fighter Sun <wanmyqawdr@126.com>
 *  NAND flash chip configuration
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <common.h>
#include <pfsv1.h>
#include <linux/err.h>

#if defined(CONFIG_JZ4780)
#include <asm/arch/jz4780_gpio.h>
#include <asm/arch/jz4780_nand.h>
#include <asm/arch/jz4780_pfsv1.h>
#elif defined(CONFIG_JZ4775)
#include <asm/arch/jz4775_gpio.h>
#include <asm/arch/jz4775_nand.h>
#include <asm/arch/jz4775_pfsv1.h>
#endif

#define GPIO_BUSY0       GPIO_PA(20)
#define GPIO_WP          GPIO_PF(22)

#define SIZE_MB          (1024 * 1024LL)
#define SIZE_ALL         (10240 * SIZE_MB)

nand_flash_if_t nand_interfaces[6] = {
    { COMMON_NAND_INTERFACE(1, GPIO_BUSY0, 1, GPIO_WP, 1) },
};

struct jz4780_nand_platform_data nand_platform_data = {
    .part_default = {
                "mtdparts=jz4780-nand:"
                "8m(xboot),"
                "16m(boot),"
                "40m(recovery),"
                "512m(system),"
                "128m(cache),"
                "1024m(data),"
                "-(misc)"
            },

    .nand_flash_if_table = nand_interfaces,
    .num_nand_flash_if = 1,

    /*
     * only single thread soft BCH ECC have
     * already got 20% speed improvement.
     *
     * TODO:
     * does some guys who handle ECC hardware implementation
     * to look at kernel soft BCH codes.
     *
     * I got the patch commits here:
     *
     * http://lists.infradead.org/pipermail/linux-mtd/2011-February/033846.html
     *
     * and a benchmark of "kernel soft BCH algorithm" VS "Chien search" on that page.
     *
     */
    .ecc_type = NAND_ECC_TYPE_HW,


    /*
     * use polled type cause speed gain
     * is about 10% ~ 15%
     *
     * use DMA cause speed gain is about 14%
     */
    .xfer_type = NAND_XFER_DMA_POLL,

    .vnand = { 1, VNAND_ECCPOS, VNAND_BADBLOCKPOS, VNAND_BADBLOCKBITS },

};


static char supported_nand[] = {
    "K9K8G08U0D\n"
    "K9GBG08U0A\n"
    "MT29F32G08CBACAWP\n"
    "MT29F64G08CBABAWP\n"
};


PFSV1_DEF_PARAM_BEGIN(NAND, "NAND subsystem",
        "text: NAND flash subsystem configuration")

    PFSV1_DEF_PARAM(nand_platform_data.part_default, "Partitions",
            char [],
            "rw",
            "text: NAND partitions in uboot.")

    PFSV1_DEF_PARAM_BLOCK_BEGIN(nand_interfaces, "PHY",
            nand_flash_if_t,
            "text: NAND flash PHY interface configuration")
        PFSV1_DEF_PARAM(nand_interfaces[0].bank, "Bank",
                int,
                "rw",
                "text: NAND is connecting bank%d\n"
                "value: [1:6]")
        PFSV1_DEF_PARAM(nand_interfaces[0].busy_gpio, "Busy pin",
                int,
                "rw",
                "text: R/#B pin\n"
                "value:"PFSV1_BRIEF_VALUE_RANGE_GPIO)
        PFSV1_DEF_PARAM(nand_interfaces[0].busy_gpio_low_assert,
                "Busy pin polarity",
                int,
                "rw", "text: R/#B pin low assert?\n"
                "value:[1(\"Yes\"), 0(\"No\")]")
        PFSV1_DEF_PARAM(nand_interfaces[0].wp_gpio, "Write protect pin",
                int,
                "rw",
                "text: Write protect pin\n"
                "value:"PFSV1_BRIEF_VALUE_RANGE_GPIO)
        PFSV1_DEF_PARAM(nand_interfaces[0].wp_gpio_low_assert,
                "Write protect pin polarity",
                int,
                "rw",
                "text: Write protect pin low assert?\n"
                "value:[1(\"Yes\"), 0(\"No\")]")
    PFSV1_DEF_PARAM_BLOCK_END(nand_interfaces)

    PFSV1_DEF_PARAM_BLOCK_BEGIN(nand_platform_data.vnand, "VNAND",
            struct ingenic_vnand, "VNAND management layer configuration")
        PFSV1_DEF_PARAM(nand_platform_data.vnand.is_vnand,
                "VNAND enable",
                int,
                "rw",
                "text: Is VNAND enabled?\n"
                "value: [0(\"No\"), 1(\"Yes\")]")
        PFSV1_DEF_PARAM(nand_platform_data.vnand.eccpos, "ECC position",
                int,
                "rw",
                "text: ECC start position in oob when using VNAND\n"
                "value: [4:]")
        PFSV1_DEF_PARAM(nand_platform_data.vnand.badblockpos,
                "Bad block position",
                int,
                "rw",
                "text: Bad block marker position in OOB when using VNAND\n"
                "value: [0:]")
        PFSV1_DEF_PARAM(nand_platform_data.vnand.badblockbits,
                "Bad block bits",
                int,
                "rw",
                "text: Maxium number of bit 1 in bad block's marker "
                "when using VNAND\n"
                "value: [0:]")
    PFSV1_DEF_PARAM_BLOCK_END(nand_platform_data.vnand)

    PFSV1_DEF_PARAM(supported_nand, "Support list",
            char [],
            "ro",
            "text: Supported NAND flash chip list")

PFSV1_DEF_PARAM_END(NAND)

PFSV1_DEF_HELP_EN_BEGIN(NAND)
    PFSV1_DEF_HELP_BLOCK_BEGIN(nand_platform_data, "")
        PFSV1_DEF_HELP(nand_platform_data.part_default,
                "Default NAND partition table\n"
                "uboot will do command: mtdparts default when it startup.")
        PFSV1_DEF_HELP(nand_platform_data.vnand.is_vnand,
                "Whether use ingenic vnand management layer or not?")
        PFSV1_DEF_HELP(nand_platform_data.vnand.eccpos,
                "ECC start position in NAND OOB area when using VNAND.\n"
                "Do not change this unless you know what you are doing!")
        PFSV1_DEF_HELP(nand_platform_data.vnand.badblockpos,
                "Bad block marker position in NAND OOB area when using"
                "VNAND.\n"
                "Do not change this unless you know what you are doing!")
        PFSV1_DEF_HELP(nand_platform_data.vnand.badblockbits,
                "Maxium number of bit 1 in bad block's marker when using"
                "VNAND.\nConsidering bit flip problem, the marker in "
                "good block may also contain bit 0.\n"
                "Do not change this unless you know what you are doing!")
    PFSV1_DEF_HELP_BLOCK_END(nand_platform_data)

    PFSV1_DEF_HELP_BLOCK_BEGIN(nand_interfaces, "")
        PFSV1_DEF_HELP(nand_interfaces[0].busy_gpio,
                "Which GPIO of processor is connecting"
                " to R/#B pin of the NAND chip")
        PFSV1_DEF_HELP(nand_interfaces[0].wp_gpio,
                "Which GPIO of processor is connecting"
                " to #WP pin of the NAND chip")
    PFSV1_DEF_HELP_BLOCK_END(nand_interfaces)
PFSV1_DEF_HELP_END(NAND)

PFSV1_DEF_HELP_ZH_BEGIN(NAND)
    PFSV1_DEF_HELP_BLOCK_BEGIN(nand_platform_data, "")
        PFSV1_DEF_HELP(nand_platform_data.part_default,
                "Uboot 启动时将创建的默认NAND分区")
    PFSV1_DEF_HELP(nand_platform_data.vnand.is_vnand,
            "是否使用ingenic vnand管理层?")
    PFSV1_DEF_HELP(nand_platform_data.vnand.eccpos,
            "使用vnand管理层时,ECC在NAND OOB区域的起始位置.\n"
            "不要修改此项，除非你清楚修改的意义!")
    PFSV1_DEF_HELP(nand_platform_data.vnand.badblockpos,
            "使用vnand管理层时,坏块标记在NAND OOB区域的位置.\n"
            "不要修改此项，除非你清楚修改的意义!")
    PFSV1_DEF_HELP(nand_platform_data.vnand.badblockbits,
            "使用vnand管理层时,一个坏块的坏块标记最大允许的bit 1的个数.\n"
            "坏块标记占128 bits, 默认当bit 1的个数小于64 bit时，认为是坏块.\n"
            "之所以如此判断，是因为对于好块也可能出现坏块标记位翻转为0的情况.\n"
            "不要修改此项，除非你清楚修改的意义!")
    PFSV1_DEF_HELP_BLOCK_END(nand_platform_data)

    PFSV1_DEF_HELP_BLOCK_BEGIN(nand_interfaces, "")
        PFSV1_DEF_HELP(nand_interfaces[0].busy_gpio,
                "设置处理器上和NAND芯片的忙等待脚相连的那个GPIO")
        PFSV1_DEF_HELP(nand_interfaces[0].busy_gpio_low_assert,
                "忙等待脚是否为低电平有效?")
        PFSV1_DEF_HELP(nand_interfaces[0].wp_gpio,
                "设置处理器上和NAND芯片写保护脚相连的那个GPIO")
        PFSV1_DEF_HELP(nand_interfaces[0].wp_gpio_low_assert,
                "写保护脚是否为低电平有效?")
    PFSV1_DEF_HELP_BLOCK_END(nand_interfaces)

    PFSV1_DEF_HELP(supported_nand,
            "NAND芯片支持列表")
PFSV1_DEF_HELP_END(nand_interfaces)
