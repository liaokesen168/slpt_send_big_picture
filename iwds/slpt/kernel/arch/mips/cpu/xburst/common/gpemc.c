/*
 *  Copyright (C) 2013 Fighter Sun <wanmyqawdr@126.com>
 *  GPEMC(NEMC) support functions
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

#include <common.h>
#include <command.h>

#include <linux/compat.h>
#include <linux/err.h>

#include <asm/bitops.h>
#include <asm/addrspace.h>
#include <asm/gpio.h>

#include <asm/arch/cpm.h>
#if defined(CONFIG_JZ4780)
#include <asm/arch/jz4780_gpio.h>
#elif defined(CONFIG_JZ4775)
#include <asm/arch/jz4775_gpio.h>
#elif defined(CONFIG_M200)
#include <asm/arch/m200_gpio.h>
#endif
#include <asm/arch/gpemc.h>

#define DRVNAME "gpemc"

#define GPEMC_REGS_FILE_BASE 	0x13410000
#define CPEMC_NAND_REGS_FILE_BASE	0x13410050

#define BANK_COUNT 7
#define GPEMC_CS1_IOBASE 0x1b000000
#define GPEMC_CS2_IOBASE 0x1a000000
#define GPEMC_CS3_IOBASE 0x19000000
#define GPEMC_CS4_IOBASE 0x18000000
#define GPEMC_CS5_IOBASE 0x17000000
#define GPEMC_CS6_IOBASE 0x16000000

#define GPEMC_NAND_BANK_DATA_OFFSET	0
#define GPEMC_NAND_BANK_ADDR_OFFSET	0x800000
#define GPEMC_NAND_BANK_CMD_OFFSET	0x400000

#define JZ_GPIO_FUNC_MEM_CS      GPIO_FUNC_0
#define JZ_GPIO_FUNC_MEM_RD_WE   GPIO_FUNC_0
#define JZ_GPIO_FUNC_MEM_FRD_FWE GPIO_FUNC_0
#define JZ_GPIO_FUNC_MEM_DATA    GPIO_FUNC_0
#define JZ_GPIO_FUNC_MEM_WAIT    GPIO_FUNC_0
#define JZ_GPIO_FUNC_MEM_ADDR    GPIO_FUNC_0

#define JZ_GPIO_FUNC_DQS        GPIO_FUNC_0

static const u32 nT_to_adjs[] = {
	/* 0 ~ 10 */
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,

	/* 11 ~ 12 */
	11, 11,

	/* 13 ~ 15*/
	12, 12, 12,

	/* 16 ~ 20 */
	13, 13, 13, 13, 13,

	/* 21 ~ 25 */
	14, 14, 14, 14, 14,

	/* 26 ~ 31 */
	15, 15, 15, 15, 15, 15
};

static const u32 adjs_to_nT[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8 ,9 ,10,
	12, 15, 20, 25, 31
};

static inline u32 div_ceiling(u32 x, u32 y)
{
	return (x + y - 1) / y;
}

typedef struct {
	volatile u32 nfcsr;

	volatile u32 pad0[(0x13410100 - CPEMC_NAND_REGS_FILE_BASE)
	                  / sizeof(u32) - 1];

	volatile u32 pncr;
	volatile u32 pndr;
	volatile u32 bitcnt;
	volatile u32 tgwe;
	volatile u32 tgcr[6];
	volatile u32 tgsr;
	volatile u32 tgfl;
	volatile u32 tgfh;
	volatile u32 tgcl;
	volatile u32 tgch;
	volatile u32 tgpd;
	volatile u32 tgsl;
	volatile u32 tgsh;
	volatile u32 tgrr;
	volatile u32 tgdr;
	volatile u32 tghl;
	volatile u32 tghh;
} nand_regs_file_t;

typedef struct {
	volatile u32 pad0[0x14 / sizeof(u32)];

	volatile u32 smcr[6];

	volatile u32 pad1;

	volatile u32 sacr[6];
} regs_file_t;

struct addr_pins_pool {
	u32 addr_pins[6];
	u32 addr_pin_use_count[6];
};

typedef struct {
	u32 start;
	u32 end;
} bank_mem_t;

/* instance a singleton gpemc */
static struct {
	const u32 gpio_pin_data[8];
	struct addr_pins_pool addr_pins_pool;
	const u32 gpio_pin_rd_we[2];
	const u32 gpio_pin_frd_fwe[2];
	const u32 gpio_pin_wait;
	const u32 gpio_pin_dqs;
	const u32 gpio_pin_cs[7];

	int use_count;

	int pin_data_use_count;
	int pin_wait_use_count;
	int pin_dqs_use_count;
	int pin_frd_fwe_use_count;
	int pin_rd_we_use_count;

	struct clk *clk;
	unsigned long long bank_use_map;

	regs_file_t * const regs_file;
	nand_regs_file_t * const nand_regs_file;


	const bank_mem_t bank_mem[BANK_COUNT];
	const char *bank_name[BANK_COUNT];
	const char *bank_type_name[CNT_BANK_TYPES];
	const char *bank_sram_type_name[CNT_SRAM_TYPES];

	gpemc_bank_t *requested_banks[BANK_COUNT];
} instance = {
	.regs_file = (regs_file_t *)
					CKSEG1ADDR(GPEMC_REGS_FILE_BASE),
	.nand_regs_file = (nand_regs_file_t *)
						CKSEG1ADDR(CPEMC_NAND_REGS_FILE_BASE),

#define GPEMC_BANK_SIZE (GPEMC_CS1_IOBASE - GPEMC_CS2_IOBASE)
	.bank_mem = {
		{},

		{
			.start = GPEMC_CS1_IOBASE,
			.end = GPEMC_CS1_IOBASE + GPEMC_BANK_SIZE - 1,
		},

		{
			.start = GPEMC_CS2_IOBASE,
			.end = GPEMC_CS2_IOBASE + GPEMC_BANK_SIZE - 1,
		},

		{
			.start = GPEMC_CS3_IOBASE,
			.end = GPEMC_CS3_IOBASE + GPEMC_BANK_SIZE - 1,
		},

		{
			.start = GPEMC_CS4_IOBASE,
			.end = GPEMC_CS4_IOBASE + GPEMC_BANK_SIZE - 1,
		},

		{
			.start = GPEMC_CS5_IOBASE,
			.end = GPEMC_CS5_IOBASE + GPEMC_BANK_SIZE - 1,
		},

		{
			.start = GPEMC_CS6_IOBASE,
			.end = GPEMC_CS6_IOBASE + GPEMC_BANK_SIZE - 1,
		}
	},

	.bank_name = {
		"",
		"gpemc-bank1-mem",
		"gpemc-bank2-mem",
		"gpemc-bank3-mem",
		"gpemc-bank4-mem",
		"gpemc-bank5-mem",
		"gpemc-bank6-mem",
	},

	.bank_type_name = {
		[BANK_TYPE_SRAM] = "SRAM",
		[BANK_TYPE_NAND] = "Common NAND",
		[BANK_TYPE_TOGGLE] = "Toggle NAND",
	},

	.bank_sram_type_name = {
		[SRAM_TYPE_NORMAL] = "Normal",
		[SRAM_TYPE_BURST] = "Burst",
	},

	.gpio_pin_data = {
		GPIO_PA(0),
		GPIO_PA(1),
		GPIO_PA(2),
		GPIO_PA(3),
		GPIO_PA(4),
		GPIO_PA(5),
		GPIO_PA(6),
		GPIO_PA(7),
	},

	.addr_pins_pool.addr_pins = {
		/*
		 * also CLE for NAND
		 */
#define PIN_INDEX_OF_CLE	0
		[PIN_INDEX_OF_CLE] = GPIO_PB(0),

		/*
		 * also ALE for NAND
		 */
#define PIN_INDEX_OF_ALE	1
		[PIN_INDEX_OF_ALE] = GPIO_PB(1),

		GPIO_PB(2),
		GPIO_PB(3),
		GPIO_PB(4),
		GPIO_PB(5),
	},

	.gpio_pin_rd_we = {
		GPIO_PA(16),
		GPIO_PA(17),
	},

	.gpio_pin_frd_fwe = {
		GPIO_PA(18),
		GPIO_PA(19),
	},

	.gpio_pin_wait = GPIO_PA(27),

	.gpio_pin_dqs = GPIO_PA(29),

	.gpio_pin_cs = {
		0,
		GPIO_PA(21),
		GPIO_PA(22),
		GPIO_PA(23),
		GPIO_PA(24),
		GPIO_PA(25),
		GPIO_PA(26),
	},

}, *gpemc = &instance; /* end instance a singleton gpemc */

static int gpemc_request_gpio_generic(const u32 *array,
		int count, enum gpio_function func,
		int *use_count, const char *name)
{
	int i;
	int ret = 0;
	char pin_name[30];

	if (*use_count == 0) {
		for (i = 0; i < count; i++) {
			sprintf(pin_name, "%s%d", name, i);
			ret = gpio_request(array[i], name);
			if (ret < 0) {
				i--;
				do {
					gpio_free(array[i]);
				} while (i--);

				printk(DRVNAME": failed to request pin %s\n", pin_name);
				goto err_return;
			}

			ret = jz_gpio_set_func(array[i], func);
			if (ret < 0) {
				do {
					gpio_free(array[i]);
				} while (i--);

				printk(DRVNAME": failed to set function for pin %s\n", pin_name);
				goto err_return;
			}
		}
	}

	(*use_count)++;

err_return:
	return ret;
}

static void gpemc_release_gpio_generic(const u32 *array,
		int count, int *use_count)
{
	int i;

	(*use_count)--;
	if (*use_count == 0) {
		for (i = 0; i < count; i++) {
			gpio_free(array[i]);
		}
	}
}

static int gpemc_request_gpio_data(void)
{
	return gpemc_request_gpio_generic(gpemc->gpio_pin_data,
			ARRAY_SIZE(gpemc->gpio_pin_data), JZ_GPIO_FUNC_MEM_DATA,
			&gpemc->pin_data_use_count, "gpemc-data");
}

static void gpemc_release_gpio_data(void)
{
	gpemc_release_gpio_generic(gpemc->gpio_pin_data,
			ARRAY_SIZE(gpemc->gpio_pin_data),
			&gpemc->pin_data_use_count);
}

static int gpemc_request_gpio_addr_one(int index, const char *name)
{
	int ret = 0;
	char pin_name[30];

	if (gpemc->addr_pins_pool.addr_pin_use_count[index] == 0) {

		sprintf(pin_name, "%s%d", name, index);
		ret = gpio_request(gpemc->addr_pins_pool.addr_pins[index],
				name);
		if (ret) {
			printk(DRVNAME": failed to request pin: %s\n", pin_name);
			goto err_return;
		}

		ret = jz_gpio_set_func(gpemc->addr_pins_pool.addr_pins[index],
				JZ_GPIO_FUNC_MEM_ADDR);
		if (ret) {
			printk(DRVNAME": failed to set function for pin: %s\n", pin_name);
			gpio_free(gpemc->addr_pins_pool.addr_pins[index]);
			goto err_return;
		}

	}

	gpemc->addr_pins_pool.addr_pin_use_count[index]++;

err_return:
	return ret;
}

static void gpemc_release_gpio_addr_one(int index)
{
	int use_count;

	use_count = --gpemc->addr_pins_pool.addr_pin_use_count[index];

	if (use_count == 0)
		gpio_free(gpemc->addr_pins_pool.addr_pins[index]);
}

static int gpemc_request_gpio_addr(int cnt_addr_pins)
{
	int i;
	int ret = 0;

	for (i = 0; i < cnt_addr_pins; i++) {
		ret = gpemc_request_gpio_addr_one(i, "gpemc-addr");
		if (ret)
			goto err_return;
	}

err_return:
	return ret;
}

static void gpemc_release_gpio_addr(int cnt_addr_pins)
{
	int i;

	for (i = 0; i < cnt_addr_pins; i++)
		gpemc_release_gpio_addr_one(i);
}


static int gpemc_request_gpio_cle_ale(void)
{
	int ret;

	ret = gpemc_request_gpio_addr_one(PIN_INDEX_OF_CLE, "gpemc-cle");
	if (ret)
		goto err_return;

	ret = gpemc_request_gpio_addr_one(PIN_INDEX_OF_ALE, "gpemc-ale");
	if (ret)
		goto err_return;

err_return:
	return ret;
}

static void gpemc_release_gpio_cle_ale(void)
{
	gpemc_release_gpio_addr_one(PIN_INDEX_OF_CLE);
	gpemc_release_gpio_addr_one(PIN_INDEX_OF_ALE);
}

static int gpemc_request_gpio_rd_we(void)
{
	return gpemc_request_gpio_generic(gpemc->gpio_pin_rd_we,
			ARRAY_SIZE(gpemc->gpio_pin_rd_we), JZ_GPIO_FUNC_MEM_RD_WE,
			&gpemc->pin_rd_we_use_count, "gpemc-rd/we");
}

static int gpemc_request_gpio_frd_fwe(void)
{
	return gpemc_request_gpio_generic(gpemc->gpio_pin_frd_fwe,
			ARRAY_SIZE(gpemc->gpio_pin_frd_fwe), JZ_GPIO_FUNC_MEM_FRD_FWE,
			&gpemc->pin_frd_fwe_use_count, "gpemc-frd/fwe");
}

static void gpemc_release_gpio_rd_we(void)
{
	gpemc_release_gpio_generic(gpemc->gpio_pin_rd_we,
			ARRAY_SIZE(gpemc->gpio_pin_rd_we),
			&gpemc->pin_rd_we_use_count);
}

static void gpemc_release_gpio_frd_fwe(void)
{
	gpemc_release_gpio_generic(gpemc->gpio_pin_frd_fwe,
			ARRAY_SIZE(gpemc->gpio_pin_frd_fwe),
			&gpemc->pin_frd_fwe_use_count);
}

static int gpemc_request_gpio_cs(int cs)
{
	int ret;

	ret = gpio_request(gpemc->gpio_pin_cs[cs], "gpemc-cs");
	if (ret < 0) {
		printk(DRVNAME": failed to request pin cs%d\n", cs);
		goto err_return;
	}

	ret = jz_gpio_set_func(gpemc->gpio_pin_cs[cs], JZ_GPIO_FUNC_MEM_CS);
	if (ret < 0) {
		gpio_free(gpemc->gpio_pin_cs[cs]);
		printk(DRVNAME": failed to set function for pin cs%d\n", cs);
		goto err_return;
	}

err_return:
	return ret;
}

static void gpemc_release_gpio_cs(int cs)
{
	gpio_free(gpemc->gpio_pin_cs[cs]);
}

/*
 * TODO: request input wait#
 */
#if 0

static int gpemc_request_gpio_wait(void)
{
	return gpemc_request_gpio_generic(&gpemc->gpio_pin_wait,
			1, JZ_GPIO_FUNC_MEM_WAIT,
			&gpemc->pin_wait_use_count, "gpemc-wait");
}

static void gpemc_release_gpio_wait(void)
{
	gpemc_release_gpio_generic(&gpemc->gpio_pin_wait,
			1, &gpemc->pin_wait_use_count);
}

#endif

static int gpemc_request_gpio_dqs(void)
{
	return gpemc_request_gpio_generic(&gpemc->gpio_pin_dqs,
			1, JZ_GPIO_FUNC_DQS,
			&gpemc->pin_rd_we_use_count, "gpemc-dqs");
}

static void gpemc_release_gpio_dqs(void)
{
	gpemc_release_gpio_generic(&gpemc->gpio_pin_dqs,
			1, &gpemc->pin_dqs_use_count);
}

static int gpemc_request_gpio_for_sram(gpemc_bank_t *bank)
{

	/*
	 * need inout  DATA[7 : 0]
	 *      output ADDR[cnt : 0]
	 *      output WE#
	 *      output RD#
	 *      output CS#
	 *
	 * TODO: request input wait#
	 */

	int ret;

	ret = gpemc_request_gpio_data();
	if (ret)
		return ret;

	if (bank->cnt_addr_pins) {
		ret = gpemc_request_gpio_addr(bank->cnt_addr_pins);
		if (ret)
			goto err_release_data;
	}

	ret = gpemc_request_gpio_rd_we();
	if (ret)
		goto err_release_addr;

	ret = gpemc_request_gpio_cs(bank->cs);
	if (ret)
		goto err_release_rd_we;

	return ret;

err_release_rd_we:
	gpemc_release_gpio_rd_we();

err_release_addr:
	gpemc_release_gpio_addr(bank->cnt_addr_pins);

err_release_data:
	gpemc_release_gpio_data();

	return ret;
}

static int gpemc_request_gpio_for_common_nand(gpemc_bank_t *bank)
{
	/*
	 * need inout  DATA[7 : 0]
	 *      output CLE, ALE
	 *      output WE#
	 *      output RD#
	 *      output CS#
	 */

	int ret;

	ret = gpemc_request_gpio_data();
	if (ret)
		return ret;

	ret = gpemc_request_gpio_cle_ale();
	if (ret)
		goto err_release_data;

	ret = gpemc_request_gpio_frd_fwe();
	if (ret)
		goto err_release_cle_ale;

	ret = gpemc_request_gpio_cs(bank->cs);
	if (ret)
		goto err_release_rd_we;

	return ret;

err_release_rd_we:
	gpemc_release_gpio_frd_fwe();

err_release_cle_ale:
	gpemc_release_gpio_cle_ale();

err_release_data:
	gpemc_release_gpio_data();

	return ret;
}

static int gpemc_request_gpio_for_toggle_nand(gpemc_bank_t *bank)
{
	/*
	 * need inout  DATA[7 : 0]
	 *      output CLE, ALE
	 *      output WE#
	 *      output RD#
	 *      output CS#
	 *      output DQS
	 */

	int ret;

	ret = gpemc_request_gpio_data();
	if (ret)
		return ret;

	ret = gpemc_request_gpio_cle_ale();
	if (ret)
		goto err_release_data;

	ret = gpemc_request_gpio_frd_fwe();
	if (ret)
		goto err_release_cle_ale;

	ret = gpemc_request_gpio_cs(bank->cs);
	if (ret)
		goto err_release_rd_we;

	ret = gpemc_request_gpio_dqs();
	if (ret)
		goto err_release_cs;

	return ret;

err_release_cs:
	gpemc_release_gpio_cs(bank->cs);

err_release_rd_we:
	gpemc_release_gpio_frd_fwe();

err_release_cle_ale:
	gpemc_release_gpio_cle_ale();

err_release_data:
	gpemc_release_gpio_data();

	return ret;
}

static void gpemc_release_gpio_for_sram(gpemc_bank_t *bank)
{
	gpemc_release_gpio_data();
	gpemc_release_gpio_rd_we();
	gpemc_release_gpio_addr(bank->cnt_addr_pins);
	gpemc_release_gpio_cs(bank->cs);
}

static void gpemc_release_gpio_for_common_nand(gpemc_bank_t *bank)
{
	gpemc_release_gpio_data();
	gpemc_release_gpio_frd_fwe();
	gpemc_release_gpio_cle_ale();
	gpemc_release_gpio_cs(bank->cs);
}

static void gpemc_release_gpio_for_toggle_nand(gpemc_bank_t *bank)
{
	gpemc_release_gpio_data();
	gpemc_release_gpio_frd_fwe();
	gpemc_release_gpio_cle_ale();
	gpemc_release_gpio_cs(bank->cs);
	gpemc_release_gpio_dqs();
}

static int gpemc_request_gpio(gpemc_bank_t *bank)
{
	int ret = 0;

	switch (bank->bank_type) {
	case BANK_TYPE_SRAM:
		ret = gpemc_request_gpio_for_sram(bank);
		break;

	case BANK_TYPE_NAND:
		ret = gpemc_request_gpio_for_common_nand(bank);
		break;

	case BANK_TYPE_TOGGLE:
		ret = gpemc_request_gpio_for_toggle_nand(bank);
		break;
	default:
		ret = -EINVAL;
		break;
	}


	return ret;
}

static void gpemc_release_gpio(gpemc_bank_t *bank)
{
	switch (bank->bank_type) {
	case BANK_TYPE_SRAM:
		gpemc_release_gpio_for_sram(bank);
		break;

	case BANK_TYPE_NAND:
		gpemc_release_gpio_for_common_nand(bank);
		break;

	case BANK_TYPE_TOGGLE:
		gpemc_release_gpio_for_toggle_nand(bank);
		break;
	default:
		break;
	}
}

void gpemc_set_bank_as_common_nand(gpemc_bank_t *bank)
{
	u32 index;

	BUG_ON(bank->bank_type != BANK_TYPE_NAND);

	/* set nand type as common nand */
	index = 16 + bank->cs - 1;
	gpemc->nand_regs_file->nfcsr &= ~BIT(index);

	/* set bank role as nand */
	index = (bank->cs - 1) << 1;
	gpemc->nand_regs_file->nfcsr |= BIT(index);

	bank->bank_type = BANK_TYPE_NAND;
}

void gpemc_set_bank_as_toggle_nand(gpemc_bank_t *bank)
{
	u32 index;

	BUG_ON(bank->bank_type != BANK_TYPE_TOGGLE);

	/* set nand type as toggle */
	index = 16 + bank->cs - 1;
	gpemc->nand_regs_file->nfcsr |= BIT(index);

	/* set bank role as nand */
	index = (bank->cs - 1) << 1;
	gpemc->nand_regs_file->nfcsr |= BIT(index);

	bank->bank_type = BANK_TYPE_TOGGLE;
}

static void gpemc_set_bank_as_sram(gpemc_bank_t *bank)
{
	u32 index;

	BUG_ON(bank->bank_type != BANK_TYPE_SRAM);

	/* set bank role as sram */
	index = (bank->cs - 1) << 1;
	gpemc->nand_regs_file->nfcsr &= ~BIT(index);

	bank->bank_type = BANK_TYPE_SRAM;
}

static void gpemc_set_bank_role(gpemc_bank_t *bank)
{
	switch (bank->bank_type) {
	case BANK_TYPE_SRAM:
		gpemc_set_bank_as_sram(bank);
		break;

	case BANK_TYPE_NAND:
		gpemc_set_bank_as_common_nand(bank);
		break;

	case BANK_TYPE_TOGGLE:
		gpemc_set_bank_as_toggle_nand(bank);
		break;
	default:
		break;
	}
}

int gpemc_request_cs(const char *dev, gpemc_bank_t *bank, int cs)
{
	int ret = 0;

	BUG_ON(cs < 1 || cs >= ARRAY_SIZE(gpemc->bank_mem));
	BUG_ON(bank->bank_type < BANK_TYPE_SRAM
			|| bank->bank_type >= CNT_BANK_TYPES);

	bank->dev = dev;
	bank->cs = cs;

	/* T(real) count in picosecond */
	bank->bank_timing.clk_T_real_ps = (1000 * 1000 * 1000) /
			(clk_get_rate(gpemc->clk) / 1000);

	if (bank->bank_type == BANK_TYPE_SRAM) {
		if (bank->cnt_addr_pins >
			ARRAY_SIZE(gpemc->addr_pins_pool.addr_pins)) {
			printk(DRVNAME": WARNING: can not allocate %u pins for address.\n",
				bank->cnt_addr_pins);

			BUG();
		}
	}

	if (test_bit(cs, &gpemc->bank_use_map)) {
		printk(DRVNAME": grab cs %d failed, it's busy.\n", cs);
		goto err_busy_bank;
	}

	ret = gpemc_request_gpio(bank);
	if (ret)
		return ret;

	set_bit(cs, &gpemc->bank_use_map);

	bank->io_base = (void *)CKSEG1ADDR(gpemc->bank_mem[cs].start);
	bank->io_nand_dat = bank->io_base + GPEMC_NAND_BANK_DATA_OFFSET;
	bank->io_nand_addr = bank->io_base + GPEMC_NAND_BANK_ADDR_OFFSET;
	bank->io_nand_cmd = bank->io_base + GPEMC_NAND_BANK_CMD_OFFSET;

	gpemc_set_bank_role(bank);

	gpemc->requested_banks[cs] = bank;

	if (gpemc->use_count == 0)
		clk_enable(gpemc->clk);
	gpemc->use_count++;

	return 0;

err_busy_bank:
	return -EBUSY;
}

void gpemc_release_cs(gpemc_bank_t* bank)
{
	BUG_ON(bank->cs < 1 || bank->cs >= ARRAY_SIZE(gpemc->bank_mem));
	if (!test_bit(bank->cs, &gpemc->bank_use_map)) {
		printk(DRVNAME": WARNING: try to release a free cs.\n");
		return;
	}

	gpemc_release_gpio(bank);

	clear_bit(bank->cs, &gpemc->bank_use_map);

	gpemc->requested_banks[bank->cs] = NULL;

	gpemc->use_count--;
	if (gpemc->use_count == 0)
		clk_disable(gpemc->clk);
}

void gpemc_fill_timing_from_nand(gpemc_bank_t *bank,
		common_nand_timing_t *timing)
{
	u32 temp;

	/* bank Tas */
	temp = max(timing->Tals, timing->Tcls);
	bank->bank_timing.sram_timing.Tas = temp;

	/* bank Tah */
	temp = max(timing->Talh, timing->Tclh);
	temp = max(temp, timing->Tch);
	temp = max(temp, timing->Tdh);
	bank->bank_timing.sram_timing.Tah = temp;

	/* bank Taw */
	temp = max(timing->Trc, timing->Twc);
	temp = max(temp - bank->bank_timing.sram_timing.Tah, timing->Trp);
	bank->bank_timing.sram_timing.Taw = temp;

	/*
	 * bank Tstrv
	 * no need to do timings plus
	 */
	temp = timing->Trhw;
	bank->bank_timing.sram_timing.Tstrv = temp;

	/* bank Tbp */
	temp = timing->Twp;
	temp = max(temp, (timing->Twc - timing->Twp));
	bank->bank_timing.sram_timing.Tbp = temp;

	if (timing->all_timings_plus >= 0) {
		bank->bank_timing.sram_timing.Tas +=
				(u32)timing->all_timings_plus;
		bank->bank_timing.sram_timing.Tah +=
				(u32)timing->all_timings_plus;
		bank->bank_timing.sram_timing.Taw +=
				(u32)timing->all_timings_plus;
		bank->bank_timing.sram_timing.Tbp +=
				(u32)timing->all_timings_plus;
	} else {
		bank->bank_timing.sram_timing.Tas -=
				(u32)abs(timing->all_timings_plus);
		bank->bank_timing.sram_timing.Tah -=
				(u32)abs(timing->all_timings_plus);
		bank->bank_timing.sram_timing.Taw -=
				(u32)abs(timing->all_timings_plus);
		bank->bank_timing.sram_timing.Tbp -=
				(u32)abs(timing->all_timings_plus);
	}

	/* bank BW */
	bank->bank_timing.BW = timing->BW;

}

void gpemc_fill_timing_from_toggle(gpemc_bank_t *bank,
		toggle_nand_timing_t *timing)
{
	/*
	 * TODO
	 */
}

void gpemc_fill_timing_from_sram(gpemc_bank_t *bank,
		sram_timing_t *timing)
{
	/*
	 * TODO
	 */
}

void gpemc_relax_bank_timing(gpemc_bank_t *bank)
{
	/*
	 * all sram timing relax to half of special cycles
	 * BW=8, BL=4, Normal sram
	 *
	 * WTF!? it's reasonless that we got corrupt data if
	 * set all special timings cycles to max
	 */
	gpemc->regs_file->smcr[bank->cs - 1] = (0xf << 24) | (0xc << 20)
			| (0xc << 16) | (0x8 << 12) | (0x8 << 8);

	/* TODO: all toggle nand timing relax  */
}

int gpemc_config_bank_timing(gpemc_bank_t *bank)
{
	u32 smcr, temp;
	u64 clk_T_real_ps = bank->bank_timing.clk_T_real_ps;

	smcr = gpemc->regs_file->smcr[bank->cs - 1];

	switch (bank->bank_type) {
	case BANK_TYPE_SRAM:
		/*
		 * TODO
		 */
		break;

	case BANK_TYPE_NAND:
		/* Tah */
		temp = div_ceiling(bank->bank_timing.sram_timing.Tah * 1000,
				clk_T_real_ps);
		if (temp > 15) {
			printk(DRVNAME": Failed to configure Tah for bank%d,"
					" Tah: %uT\n", bank->cs, temp);
			return -EINVAL;
		}
		smcr &= ~(0xf << 12);
		smcr |= temp << 12;

		/* Taw */
		temp = div_ceiling(bank->bank_timing.sram_timing.Taw * 1000,
				clk_T_real_ps);
		if (temp > 31) {
			printk(DRVNAME": Failed to configure Taw for bank%d,"
					" Taw: %uT\n", bank->cs, temp);
			return -EINVAL;
		}
		temp = nT_to_adjs[temp];
		smcr &= ~(0xf << 20);
		smcr |= temp << 20;

		/* Tas */
		temp = div_ceiling(bank->bank_timing.sram_timing.Tas * 1000,
				clk_T_real_ps);
		if (temp > 15) {
			printk(DRVNAME": Failed to configure Tas for bank%d,"
					" Tas: %uT\n", bank->cs, temp);
			return -EINVAL;
		}
		smcr &= ~(0xf << 8);
		smcr |= temp << 8;

		/* Tstrv */
		temp = div_ceiling(bank->bank_timing.sram_timing.Tstrv * 1000,
				clk_T_real_ps);
		if (temp > 63) {
			printk(DRVNAME": Failed to configure Tstrv for bank%d,"
					" Tstrv: %uT\n", bank->cs, temp);
			return -EINVAL;
		}
		smcr &= ~(0x3f << 24);
		smcr |= temp << 24;

		/* Tbp */
		temp = div_ceiling(bank->bank_timing.sram_timing.Tbp * 1000,
				clk_T_real_ps);
		if (temp > 31) {
			printk(DRVNAME": Failed to configure Tbp for bank%d,"
					" Tbp: %uT\n", bank->cs, temp);
			return -EINVAL;
		}
		temp = nT_to_adjs[temp];
		smcr &= ~(0xf << 16);
		smcr |= temp << 16;

		gpemc->regs_file->smcr[bank->cs - 1] = smcr;

		break;

	case BANK_TYPE_TOGGLE:
		/*
		 * TODO
		 */
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

int gpemc_config_toggle_bank_timing(gpemc_bank_t *bank)
{

	/*
	 * TODO
	 */
	return 0;
}

void gpemc_enable_nand_flash(gpemc_bank_t *bank, bool enable)
{
	u32 index;

	index = ((bank->cs - 1) << 1) + 1;
	if (enable)
		gpemc->nand_regs_file->nfcsr |= BIT(index);
	else
		gpemc->nand_regs_file->nfcsr &= ~BIT(index);
}

void gpemc_enable_pn(bool enable)
{
	if (enable)
		gpemc->nand_regs_file->pncr = 0x3;
	else
		gpemc->nand_regs_file->pncr = 0;
}

int gpemc_init(void)
{
	gpemc->clk = clk_get("nemc");
	if (IS_ERR(gpemc->clk)) {
		printk(DRVNAME": failed to get gpemc clock.\n");
		return PTR_ERR(gpemc->clk);
	}

	printk(DRVNAME": SoC-jz4780 gpemc support functions initilized.\n");

	return 0;
}

static int
gpemc_debugfs_show(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 smcr;
	u64 temp;
	int cs;
	gpemc_bank_t *bank;

	for (cs = 1; cs < BANK_COUNT; cs++) {
		if (test_bit(cs, &gpemc->bank_use_map)) {
			bank = gpemc->requested_banks[cs];
			smcr = gpemc->regs_file->smcr[cs - 1];

			printk("\n");
			printk("====== Bank%d ======\n", cs);
			printk("Owner: %s\n", bank->dev);
			printk("Type: %s\n",
					gpemc->bank_type_name[bank->bank_type]);
			printk("Clk: %luHz\n", clk_get_rate(gpemc->clk));
			printk("clk_T(real): %llups\n",
					bank->bank_timing.clk_T_real_ps);
			printk("BW: %dbit\n", bank->bank_timing.BW);

			switch (bank->bank_type) {
			case BANK_TYPE_SRAM:
				/*
				 * TODO
				 */
				break;
			case BANK_TYPE_NAND:
			{

				printk("\n");
				printk("Timings loaded from NAND chip\n");
				printk("-----------------------------\n");
				printk("Tstrv: %uns\n",
						bank->bank_timing.sram_timing.Tstrv);

				printk("Taw: %uns\n",
						bank->bank_timing.sram_timing.Taw);
				printk("Tbp: %uns\n",
						bank->bank_timing.sram_timing.Tbp);
				printk("Tah: %uns\n",
						bank->bank_timing.sram_timing.Tah);
				printk("Tas: %uns\n",
						bank->bank_timing.sram_timing.Tas);
				break;
			}
			case BANK_TYPE_TOGGLE:
				/*
				 * TODO
				 */
				break;
			default:
				break;
			}

			printk("\n");
			printk("Timings configured to bank%d\n", cs);
			printk("-----------------------------\n");
			printk("Reg SMCR: [0x%p]=0x%x\n",
					&gpemc->regs_file->smcr[cs - 1],
					smcr);

			temp = (smcr & (0x3f << 24)) >> 24;
			printk("Tstrv: %lluT(%llups)\n",
					temp, temp * bank->bank_timing.clk_T_real_ps);

			temp = adjs_to_nT[(smcr & (0xf << 20)) >> 20];
			printk("Taw: %lluT(%llups)\n",
					temp, temp * bank->bank_timing.clk_T_real_ps);

			temp = adjs_to_nT[(smcr & (0xf << 16)) >> 16];
			printk("Tbp: %lluT(%llups)\n",
					temp, temp * bank->bank_timing.clk_T_real_ps);

			temp = (smcr & (0xf << 12)) >> 12;
			printk("Tah: %lluT(%llups)\n",
					temp, temp * bank->bank_timing.clk_T_real_ps);

			temp = (smcr & (0xf << 8)) >> 8;
			printk("Tas: %lluT(%llups)\n",
					temp, temp * bank->bank_timing.clk_T_real_ps);
		}
	}

	return 0;
}

U_BOOT_CMD(
	gpemcinfo, 1, 1, gpemc_debugfs_show,
	"show GPEMC info",
	"cmd: gpemcinfo"
);

/*
 * MODULE_AUTHOR("Fighter Sun <wanmyqawdr@126.com>");
 * MODULE_DESCRIPTION("SoC-jz4780 GPEMC(NEMC) support functions");
 * MODULE_LICENSE("GPL");
 */
