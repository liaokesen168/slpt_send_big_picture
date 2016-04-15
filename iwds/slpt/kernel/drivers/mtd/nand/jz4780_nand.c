/*
 *  Copyright (C) 2013 Fighter Sun <wanmyqawdr@126.com>
 *  JZ4780 SoC NAND controller driver
 *
 *  TODO:
 *
 *  <Performance>
 *  block cache with IO prefetch and LRU replacement scheme
 *
 *  <Stability>
 *  PN random process
 *  Read retry
 *
 *  <Function>
 *  support toggle NAND
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
#include <malloc.h>

#include <linux/compat.h>
#include <linux/err.h>
#include <nand.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand_bch.h>
#include <linux/bch.h>

#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/gpemc.h>
#include <asm/arch/bch.h>
#if defined(CONFIG_JZ4780)
#include <asm/arch/jz4780_dma.h>
#include <asm/arch/jz4780_nand.h>
#elif defined(CONFIG_JZ4775)
#include <asm/arch/jz4775_dma.h>
#include <asm/arch/jz4775_nand.h>
#endif
#include <asm/arch/board_special.h>

#define DRVNAME "jz4780-nand"

#define MAX_RB_DELAY_US             50
#define MAX_RB_TIMOUT_MS            50
#define MAX_RESET_DELAY_MS          50

#define MAX_DMA_TRANSFER_TIMOUT_MS  5000
#define DMA_BUF_SIZE                PAGE_SIZE * 4

#define MAX_SWECC_REQ_POLL_TIME_MS  5

#define ONFI_SUBFEATURE_PARAM_LEN	4

struct jz4780_nand_dma {
	jz4780_dma_chan_t *chan;
	dma_slave_config_t cfg;

	jz4780_dma_req_type_t type;
};

struct jz4780_nand {
	struct mtd_info *mtd;

	struct nand_chip chip;
	struct nand_ecclayout ecclayout;

	nand_flash_if_t *nand_flash_if_table[MAX_NUM_NAND_IF];
	int num_nand_flash_if;
	int curr_nand_flash_if;

	bch_request_t bch_req;
	nand_ecc_type_t ecc_type;

	struct nand_flash_dev *nand_flash_table;
	int num_nand_flash;

	struct jz4780_nand_dma dma_pipe_nand;

	nand_flash_info_t *curr_nand_flash_info;

	nand_xfer_type_t xfer_type;
	int use_dma;

	int devnum;

	struct jz4780_nand_platform_data *pdata;
};

const char *label_wp_gpio[] = {
	DRVNAME"-THIS-IS-A-BUG",
	"bank1-nand-wp",
	"bank2-nand-wp",
	"bank3-nand-wp",
	"bank4-nand-wp",
	"bank5-nand-wp",
	"bank6-nand-wp",
};

const char *label_busy_gpio[] = {
	DRVNAME"-THIS-IS-A-BUG",
	"bank1-nand-busy",
	"bank2-nand-busy",
	"bank3-nand-busy",
	"bank4-nand-busy",
	"bank5-nand-busy",
	"bank6-nand-busy",
};

#ifdef CONFIG_DEBUG_FS

/* root entry to debug */
static struct dentry *debugfs_root;

#endif

/*
 * ******************************************************
 * 	NAND flash chip name & ID
 * ******************************************************
 */

#define NAND_FLASH_K9K8G08U0D_NAME           "K9K8G08U0D"
#define NAND_FLASH_K9K8G08U0D_ID             0xd3

/*
 * !!!Caution
 * "K9GBG08U0A" may be with one of two ID sequences:
 * "EC D7 94 76" --- this one can not be detected properly
 *
 * "EC D7 94 7A" --- this one can be detected properly
 */
#define NAND_FLASH_K9GBG08U0A_NANE           "K9GBG08U0A"
#define NAND_FLASH_K9GBG08U0A_ID             0xd7


/*
 * Detected by rules of ONFI v2.2
 */
#define NAND_FLASH_MT29F32G08CBACAWP_NAME    "MT29F32G08CBACAWP"
#define NAND_FLASH_MT29F32G08CBACAWP_ID      0x68


/*
 * Detected by rules of ONFI v2.3
 */
#define NAND_FLASH_MT29F64G08CBABAWP_NAME    "MT29F64G08CBABAWP"
#define NAND_FLASH_MT29F64G08CBABAWP_ID      0x64


/*
 * ******************************************************
 * 	Supported NAND flash chips
 * ******************************************************
 *
 */
static struct nand_flash_dev builtin_nand_flash_table[] = {
	/*
	 * These are the new chips with large page size. The pagesize and the
	 * erasesize is determined from the extended id bytes
	 */

	/*
	 * K9K8G08U0D
	 */
	{
		NAND_FLASH_K9K8G08U0D_NAME, NAND_FLASH_K9K8G08U0D_ID,
		0, 1024, 0, LP_OPTIONS
	},


	/*
	 * K9GBG08U0A
	 *
	 * !!!Caution
	 * please do not use busy pin IRQ over "K9GBG08U0A"
	 * the chip is running under very rigorous timings
	 */
	{
		NAND_FLASH_K9GBG08U0A_NANE, NAND_FLASH_K9GBG08U0A_ID,
		0, 4096, 0, LP_OPTIONS
	},


	/*
	 * MT29F32G08CBACA(WP) --- support ONFI v2.2
	 *
	 * it was detected by rules of ONFI v2.2
	 * so you can remove this match entry
	 *
	 */
	{
		NAND_FLASH_MT29F32G08CBACAWP_NAME, NAND_FLASH_MT29F32G08CBACAWP_ID,
		0, 4096, 0, LP_OPTIONS
	},


	/*
	 * MT29F32G08CBACA(WP) --- support ONFI v2.3
	 *
	 * it was detected by rules of ONFI v2.3
	 * so you can remove this match entry
	 *
	 */
	{
		NAND_FLASH_MT29F64G08CBABAWP_NAME, NAND_FLASH_MT29F64G08CBABAWP_ID,
		0, 8192, 0, LP_OPTIONS
	},


	{NULL,}
};


/*
 * ******************************************************
 * 	Supported NAND flash chips' timings parameters table
 * 	it extents the upper table
 * ******************************************************
 */
static nand_flash_info_t builtin_nand_info_table[] = {
	{
		/*
		 * Datasheet of K9K8G08U0D, Rev-0.2, P4, S1.2
		 * ECC : 1bit/528Byte
		 */
		COMMON_NAND_CHIP_INFO(
			NAND_FLASH_K9K8G08U0D_NAME,
			NAND_MFR_SAMSUNG, NAND_FLASH_K9K8G08U0D_ID,
			512, 8, 0,
			12, 5, 12, 5, 20, 5, 12, 5, 12, 10,
			25, 25, 70, 70, 100, 60, 60, 12, 20, 0, 100,
			100, 500 * 1000, 0, 0, 0, 0, BUS_WIDTH_8,
			CAN_NOT_ADJUST_OUTPUT_STRENGTH,
			CAN_NOT_ADJUST_RB_DOWN_STRENGTH,
			samsung_nand_pre_init)
	},

	{
		/*
		 * Datasheet of K9GBG08U0A, Rev-1.3, P5, S1.2
		 * ECC : 24bit/1KB
		 */
		COMMON_NAND_CHIP_INFO(
			NAND_FLASH_K9GBG08U0A_NANE,
			NAND_MFR_SAMSUNG, NAND_FLASH_K9GBG08U0A_ID,
			1024, 24,

			/*
			 * all timings adjust to +10ns
			 *
			 * we change this parameters
			 * because mtd_torturetest failed
			 * ******************************
			 */
			10,

			12, 5, 12, 5, 20, 5, 12, 5, 12, 10,
			25, 25, 300, 300, 100, 120, 300, 12, 20, 300, 100,
			100, 200 * 1000, 1 * 1000, 200 * 1000,
			5 * 1000 * 1000, 0, BUS_WIDTH_8,
			NAND_OUTPUT_NORMAL_DRIVER,
			CAN_NOT_ADJUST_RB_DOWN_STRENGTH,
			samsung_nand_pre_init)
	},

	{
		/*
		 * Datasheet of MT29F32G08CBACA(WP), Rev-E, P109, Table-17
		 * ECC : 24bit/1080bytes
		 */
		COMMON_NAND_CHIP_INFO(
			NAND_FLASH_MT29F32G08CBACAWP_NAME,
			NAND_MFR_MICRON, NAND_FLASH_MT29F32G08CBACAWP_ID,
			1024, 30, 0,
			10, 5, 10, 5, 15, 5, 7, 5, 10, 7,
			20, 20, 70, 200, 100, 60, 200, 10, 20, 0, 100,
			100, 100 * 1000, 0, 0, 0, 5, BUS_WIDTH_8,
			NAND_OUTPUT_NORMAL_DRIVER,
			NAND_RB_DOWN_FULL_DRIVER,
			micron_nand_pre_init)
	},

	{
		/*
		 * Datasheet of MT29F64G08CBABA(WP), Rev-G, P119, Table-19
		 * ECC : 40bit/1117bytes
		 *
		 * TODO: need read retry
		 *
		 */
		COMMON_NAND_CHIP_INFO(
			NAND_FLASH_MT29F64G08CBABAWP_NAME,
			NAND_MFR_MICRON, NAND_FLASH_MT29F64G08CBABAWP_ID,
			1024, 48, 0,
			10, 5, 10, 5, 15, 5, 7, 5, 10, 7,
			20, 20, 70, 200, 100, 60, 200, 10, 20, 0, 100,
			100, 100 * 1000, 1000, 0, 0, 5, BUS_WIDTH_8,
			NAND_OUTPUT_NORMAL_DRIVER,
			NAND_RB_DOWN_FULL_DRIVER,
			micron_nand_pre_init)
	},
};



/*
 * ******************************************************
 * 	NAND chip post initialize callbacks
 * ******************************************************
 */
int micron_nand_pre_init(struct jz4780_nand *nand)
{
	struct nand_chip *chip = &nand->chip;
	struct mtd_info *mtd = nand->mtd;
	nand_flash_info_t *nand_info = nand->curr_nand_flash_info;
	int ret = 0;
	int i;

	if (chip->onfi_version) {
		/*
		 * ONFI NAND
		 */

		uint8_t subfeature_param[ONFI_SUBFEATURE_PARAM_LEN];

		for (i = 0; i < nand->num_nand_flash_if; i++) {
			chip->select_chip(mtd, i);

			/*
			 * set async interface under a special timing mode
			 */
			memset(subfeature_param, 0, sizeof(subfeature_param));
			subfeature_param[0] =
					(uint8_t)nand_info->onfi_special.timing_mode;
			ret = chip->onfi_set_features(mtd, chip,
					0x1, subfeature_param);
			if (ret) {
				chip->select_chip(mtd, -1);
				goto err_return;
			}

			/*
			 * set output driver strength
			 */
			if (nand_info->output_strength !=
					CAN_NOT_ADJUST_OUTPUT_STRENGTH) {
				memset(subfeature_param, 0, sizeof(subfeature_param));
				switch (nand_info->output_strength) {
				case NAND_OUTPUT_NORMAL_DRIVER:
					subfeature_param[0] = 0x2;
					break;

				case NAND_OUTPUT_UNDER_DRIVER1:
				case NAND_OUTPUT_UNDER_DRIVER2:
					subfeature_param[0] = 0x3;
					break;

				case NAND_OUTPUT_OVER_DRIVER1:
					subfeature_param[0] = 0x1;
					break;

				case NAND_OUTPUT_OVER_DRIVER2:
					subfeature_param[0] = 0x0;
					break;

				case CAN_NOT_ADJUST_OUTPUT_STRENGTH:
					BUG();
				}

				ret = chip->onfi_set_features(mtd, chip,
						0x10, subfeature_param);
				if (ret) {
					chip->select_chip(mtd, -1);
					goto err_return;
				}
			}

			/*
			 * set R/B# pull-down strength
			 */
			if (nand_info->rb_down_strength !=
					CAN_NOT_ADJUST_RB_DOWN_STRENGTH) {
				memset(subfeature_param, 0, sizeof(subfeature_param));
				switch (nand_info->rb_down_strength) {
				case NAND_RB_DOWN_FULL_DRIVER:
					subfeature_param[0] = 0x0;
					break;

				case NAND_RB_DOWN_THREE_QUARTER_DRIVER:
					subfeature_param[0] = 0x1;
					break;

				case NAND_RB_DOWN_ONE_HALF_DRIVER:
					subfeature_param[0] = 0x2;
					break;

				case NAND_RB_DOWN_ONE_QUARTER_DRIVER:
					subfeature_param[0] = 0x3;
					break;

				case CAN_NOT_ADJUST_RB_DOWN_STRENGTH:
					BUG();
				}

				ret = chip->onfi_set_features(mtd, chip,
						0x81, subfeature_param);
				if (ret) {
					chip->select_chip(mtd, -1);
					goto err_return;
				}
			}

			chip->select_chip(mtd, -1);
		}
	} else {
		/*
		 * not ONFI NAND
		 */
	}

err_return:
	return ret;
}

int samsung_nand_pre_init(struct jz4780_nand *nand)
{
	struct nand_chip *chip = &nand->chip;
	struct mtd_info *mtd = nand->mtd;
	nand_flash_info_t *nand_info = nand->curr_nand_flash_info;
	int ret = 0;
	int i;

	if (chip->onfi_version) {
		/*
		 * ONFI NAND
		 */
	} else {
		/*
		 * not ONFI NAND
		 */

		/*
		 * even it's not a ONFI NAND but
		 * it's with the same commands set
		 */
		uint8_t subfeature_param[ONFI_SUBFEATURE_PARAM_LEN];

		for (i = 0; i < nand->num_nand_flash_if; i++) {
			chip->select_chip(mtd, i);
			/*
			 * set output driver strength
			 */
			if (nand_info->output_strength !=
					CAN_NOT_ADJUST_OUTPUT_STRENGTH) {
				memset(subfeature_param, 0, sizeof(subfeature_param));
				switch (nand_info->output_strength) {
				case NAND_OUTPUT_NORMAL_DRIVER:
					subfeature_param[0] = 0x4;
					break;

				case NAND_OUTPUT_UNDER_DRIVER1:
				case NAND_OUTPUT_UNDER_DRIVER2:
					subfeature_param[0] = 0x2;
					break;

				case NAND_OUTPUT_OVER_DRIVER1:
				case NAND_OUTPUT_OVER_DRIVER2:
					subfeature_param[0] = 0x6;
					break;

				case CAN_NOT_ADJUST_OUTPUT_STRENGTH:
					BUG();
				}

				ret = chip->onfi_set_features(mtd, chip,
						0x10, subfeature_param);
				/*
				 * Samsung does not support
				 * set_feature return value check
				 */
				ret = 0;
			}

			chip->select_chip(mtd, -1);
		}
	}

	return ret;
}


static inline struct jz4780_nand *mtd_to_jz4780_nand(struct mtd_info *mtd)
{
	return container_of(mtd->priv, struct jz4780_nand, chip);
}

static inline int jz4780_nand_chip_is_ready(struct jz4780_nand *nand,
		nand_flash_if_t *nand_if)
{
	int low_assert;
	int gpio;

	low_assert = nand_if->busy_gpio_low_assert;
	gpio = nand_if->busy_gpio;

	return !(gpio_get_value(gpio) ^ low_assert);
}

static void jz4780_nand_enable_wp(nand_flash_if_t *nand_if, int enable)
{
	int low_assert;
	int gpio;

	low_assert = nand_if->wp_gpio_low_assert;
	gpio = nand_if->wp_gpio;

	if (enable)
		gpio_set_value(gpio, low_assert ^ 1);
	else
		gpio_set_value(gpio, !(low_assert ^ 1));
}

static int jz4780_nand_dev_is_ready(struct mtd_info *mtd)
{
	struct jz4780_nand *nand;
	nand_flash_if_t *nand_if;

	int ret = 0;

	nand = mtd_to_jz4780_nand(mtd);
	nand_if = nand->nand_flash_if_table[nand->curr_nand_flash_if];

	if (nand_if->busy_gpio != -1) {
		ret = jz4780_nand_chip_is_ready(nand, nand_if);
	} else {
		udelay(MAX_RB_DELAY_US);
		ret = 1;
	}

	/*
	 * Apply this short delay always
	 * to ensure that we do wait tRR in
	 * any case on any machine.
	 */
	ndelay(100);

	return ret;
}

static void jz4780_nand_select_chip(struct mtd_info *mtd, int chip)
{
	struct nand_chip *this;
	struct jz4780_nand *nand;
	nand_flash_if_t *nand_if;

	this = mtd->priv;
	nand = mtd_to_jz4780_nand(mtd);

	if (chip == -1) {
		/*
		 * Apply this short delay always
		 * to ensure that we do wait tCH in
		 * any case on any machine.
		 */
		ndelay(100);

		/* deselect current NAND flash chip */
		nand_if =
			nand->nand_flash_if_table[nand->curr_nand_flash_if];
		gpemc_enable_nand_flash(&nand_if->cs, 0);
	} else {
		/* select new NAND flash chip */
		nand_if = nand->nand_flash_if_table[chip];
		gpemc_enable_nand_flash(&nand_if->cs, 1);

		this->IO_ADDR_R = nand_if->cs.io_nand_dat;
		this->IO_ADDR_W = nand_if->cs.io_nand_dat;

		/* reconfigure DMA */
		if (nand->use_dma &&
				nand->curr_nand_flash_if != chip) {
			nand->dma_pipe_nand.cfg.src_addr =
					(dma_addr_t)CPHYSADDR(this->IO_ADDR_R);
			nand->dma_pipe_nand.cfg.dst_addr =
					(dma_addr_t)CPHYSADDR(this->IO_ADDR_W);
		}

		nand->curr_nand_flash_if = chip;

		/*
		 * Apply this short delay always
		 * to ensure that we do wait tCS in
		 * any case on any machine.
		 */
		ndelay(100);
	}
}

static void
jz4780_nand_cmd_ctrl(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	struct jz4780_nand *nand;
	nand_flash_if_t *nand_if;

	if (cmd != NAND_CMD_NONE) {
		nand = mtd_to_jz4780_nand(mtd);
		nand_if = nand->nand_flash_if_table[nand->curr_nand_flash_if];

		if (ctrl & NAND_CLE) {
			writeb(cmd, nand_if->cs.io_nand_cmd);
		} else if (ctrl & NAND_ALE) {
			writeb(cmd, nand_if->cs.io_nand_addr);
		}
	}
}

static void jz4780_nand_ecc_hwctl(struct mtd_info *mtd, int mode)
{
	/*
	 * TODO: need consider NAND r/w state ?
	 */
}


static inline void
jz4780_nand_delay_after_command(struct jz4780_nand *nand,
		nand_flash_info_t *nand_info,
		unsigned int command)
{
	switch (command) {
	case NAND_CMD_RNDIN:
		/*
		 * Apply this short delay to meet Tcwaw
		 * some Samsung NAND chips need Tcwaw before
		 * address cycles
		 */
		if (nand_info->type == BANK_TYPE_NAND)
			ndelay(nand_info->nand_timing.
					common_nand_timing.busy_wait_timing.Tcwaw);
		else {
			/*
			 * TODO
			 * implement Tcwaw delay
			 */
		}

		break;

	case NAND_CMD_STATUS:
		/*
		 * Apply this short delay to meet Twhr
		 */
		if (nand_info->type == BANK_TYPE_NAND)
			ndelay(nand_info->nand_timing.
					common_nand_timing.busy_wait_timing.Twhr);
		else {
			/*
			 * TODO
			 * implement Tadl delay
			 */
		}

		break;

	case NAND_CMD_RNDOUTSTART:
		/*
		 * Apply this short delay to meet Twhr2
		 */
		if (nand_info->type == BANK_TYPE_NAND)
			ndelay(nand_info->nand_timing.
					common_nand_timing.busy_wait_timing.Twhr2);
		else {
			/*
			 * TODO
			 * implement Twhr2 delay
			 */
		}

		break;

	default:
		break;
	}
}

static inline void
jz4780_nand_delay_after_address(struct jz4780_nand *nand,
		nand_flash_info_t *nand_info,
		unsigned int command)
{
	switch (command) {
	case NAND_CMD_READID:
		/*
		 * Apply this short delay
		 * always to ensure that we do wait Twhr in
		 * any case on any machine.
		 */
		udelay(nand->chip.chip_delay);

		break;

	case NAND_CMD_SET_FEATURES:
	case NAND_CMD_SEQIN:
		/*
		 * Apply this short delay to meet Tadl
		 */
		if (nand_info->type == BANK_TYPE_NAND)
			ndelay(nand_info->nand_timing.
					common_nand_timing.busy_wait_timing.Tadl);
		else {
			/*
			 * TODO
			 * implement Tadl delay
			 */
		}

		break;

	case NAND_CMD_RNDIN:
		/*
		 * Apply this short delay to meet Tccs
		 */
		if (nand_info->type == BANK_TYPE_NAND)
			ndelay(nand_info->nand_timing.
					common_nand_timing.busy_wait_timing.Tccs);
		else {
			/*
			 * TODO
			 * implement Tccs delay
			 */
		}

		break;

	default:
		break;
	}
}

static void jz4780_nand_command(struct mtd_info *mtd, unsigned int command,
			 int column, int page_addr)
{
	register struct nand_chip *chip = mtd->priv;
	int ctrl = NAND_CTRL_CLE | NAND_CTRL_CHANGE;

	struct jz4780_nand *nand;
	nand_flash_if_t *nand_if;
	nand_flash_info_t *nand_info;

	nand = mtd_to_jz4780_nand(mtd);

	nand_if = nand->nand_flash_if_table[nand->curr_nand_flash_if];
	nand_if->curr_command = command;

	nand_info = nand->curr_nand_flash_info;


	/* Write out the command to the device */
	if (command == NAND_CMD_SEQIN) {
		int readcmd;

		if (column >= mtd->writesize) {
			/* OOB area */
			column -= mtd->writesize;
			readcmd = NAND_CMD_READOOB;
		} else if (column < 256) {
			/* First 256 bytes --> READ0 */
			readcmd = NAND_CMD_READ0;
		} else {
			column -= 256;
			readcmd = NAND_CMD_READ1;
		}
		chip->cmd_ctrl(mtd, readcmd, ctrl);
		ctrl &= ~NAND_CTRL_CHANGE;
	}
	chip->cmd_ctrl(mtd, command, ctrl);

	jz4780_nand_delay_after_command(nand, nand_info, command);

	/* Address cycle, when necessary */
	ctrl = NAND_CTRL_ALE | NAND_CTRL_CHANGE;
	/* Serially input address */
	if (column != -1) {
		chip->cmd_ctrl(mtd, column, ctrl);
		ctrl &= ~NAND_CTRL_CHANGE;
	}

	if (page_addr != -1) {
		chip->cmd_ctrl(mtd, page_addr, ctrl);
		ctrl &= ~NAND_CTRL_CHANGE;
		chip->cmd_ctrl(mtd, page_addr >> 8, ctrl);
		/* One more address cycle for devices > 32MiB */
		if (chip->chipsize > (32 << 20))
			chip->cmd_ctrl(mtd, page_addr >> 16, ctrl);
	}

	jz4780_nand_delay_after_address(nand, nand_info, command);

	chip->cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);

	/*
	 * Program and erase have their own
	 * busy handlers status and sequential
	 * in needs no delay
	 */
	switch (command) {

	case NAND_CMD_PAGEPROG:
	case NAND_CMD_ERASE1:
	case NAND_CMD_ERASE2:
	case NAND_CMD_SEQIN:
	case NAND_CMD_STATUS:
		return;

	case NAND_CMD_RESET:
		if (chip->dev_ready) {
			/*
			 * Apply this short delay always
			 * to ensure that we do wait tRST in
			 * any case on any machine.
			 */
			mdelay(MAX_RESET_DELAY_MS);
			break;
		}

		mdelay(MAX_RESET_DELAY_MS);

		chip->cmd_ctrl(mtd, NAND_CMD_STATUS,
			       NAND_CTRL_CLE | NAND_CTRL_CHANGE);

		jz4780_nand_delay_after_command(nand, nand_info, command);

		chip->cmd_ctrl(mtd,
			       NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
		while (!(chip->read_byte(mtd) & NAND_STATUS_READY))
				;
		return;

		/* This applies to read commands */
	default:
		/*
		 * If we don't have access
		 * to the busy pin, we apply the given
		 * command delay
		 */
		if (!chip->dev_ready) {
			udelay(chip->chip_delay);
			return;
		}
	}
	/*
	 * Apply this short delay
	 * always to ensure that we do wait tWB in
	 * any case on any machine.
	 */
	ndelay(100);

	nand_wait_ready(mtd);
}

static void jz4780_nand_command_lp(struct mtd_info *mtd,
		unsigned int command, int column, int page_addr)
{
	register struct nand_chip *chip = mtd->priv;

	struct jz4780_nand *nand;
	nand_flash_if_t *nand_if;
	nand_flash_info_t *nand_info;

	nand = mtd_to_jz4780_nand(mtd);
	nand_if = nand->nand_flash_if_table[nand->curr_nand_flash_if];
	nand_if->curr_command = command;

	nand_info = nand->curr_nand_flash_info;

	/* Emulate NAND_CMD_READOOB */
	if (command == NAND_CMD_READOOB) {
		column += mtd->writesize;
		command = NAND_CMD_READ0;
	}

	/* Command latch cycle */
	chip->cmd_ctrl(mtd, command & 0xff,
		       NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);

	jz4780_nand_delay_after_command(nand, nand_info, command);

	if (column != -1 || page_addr != -1) {
		int ctrl = NAND_CTRL_CHANGE | NAND_NCE | NAND_ALE;

		/* Serially input address */
		if (column != -1) {
			chip->cmd_ctrl(mtd, column, ctrl);
			ctrl &= ~NAND_CTRL_CHANGE;
			chip->cmd_ctrl(mtd, column >> 8, ctrl);
		}
		if (page_addr != -1) {
			chip->cmd_ctrl(mtd, page_addr, ctrl);
			chip->cmd_ctrl(mtd, page_addr >> 8,
				       NAND_NCE | NAND_ALE);
			/* One more address cycle for devices > 128MiB */
			if (chip->chipsize > (128 << 20))
				chip->cmd_ctrl(mtd, page_addr >> 16,
					       NAND_NCE | NAND_ALE);

		}
	}

	jz4780_nand_delay_after_address(nand, nand_info, command);

	chip->cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);

	/*
	 * Program and erase have their own
	 * busy handlers status, sequential
	 * in, and deplete1 need no delay.
	 */
	switch (command) {

	case NAND_CMD_CACHEDPROG:
	case NAND_CMD_PAGEPROG:
	case NAND_CMD_ERASE1:
	case NAND_CMD_ERASE2:
	case NAND_CMD_SEQIN:
	case NAND_CMD_RNDIN:
	case NAND_CMD_STATUS:
	case NAND_CMD_DEPLETE1:
		return;

	case NAND_CMD_STATUS_ERROR:
	case NAND_CMD_STATUS_ERROR0:
	case NAND_CMD_STATUS_ERROR1:
	case NAND_CMD_STATUS_ERROR2:
	case NAND_CMD_STATUS_ERROR3:
		/*
		 * Read error status commands
		 * require only a short delay
		 */
		udelay(chip->chip_delay);
		return;

	case NAND_CMD_RESET:
		if (chip->dev_ready) {
			/*
			 * Apply this short delay always to
			 * ensure that we do wait tRST in
			 * any case on any machine.
			 */
			mdelay(MAX_RESET_DELAY_MS);
			break;
		}

		mdelay(MAX_RESET_DELAY_MS);

		chip->cmd_ctrl(mtd, NAND_CMD_STATUS,
			       NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);

		jz4780_nand_delay_after_command(nand, nand_info, command);

		chip->cmd_ctrl(mtd, NAND_CMD_NONE,
			       NAND_NCE | NAND_CTRL_CHANGE);
		while (!(chip->read_byte(mtd) & NAND_STATUS_READY))
				;
		return;

	case NAND_CMD_RNDOUT:
		/* No ready / busy check necessary */
		chip->cmd_ctrl(mtd, NAND_CMD_RNDOUTSTART,
			       NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);

		jz4780_nand_delay_after_command(nand, nand_info, command);

		chip->cmd_ctrl(mtd, NAND_CMD_NONE,
			       NAND_NCE | NAND_CTRL_CHANGE);
		return;

	case NAND_CMD_READ0:
		chip->cmd_ctrl(mtd, NAND_CMD_READSTART,
			       NAND_NCE | NAND_CLE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd, NAND_CMD_NONE,
			       NAND_NCE | NAND_CTRL_CHANGE);

		/* This applies to read commands */
	default:
		/*
		 * If we don't have access to the busy pin, we apply the given
		 * command delay.
		 */
		if (!chip->dev_ready) {
			udelay(chip->chip_delay);
			return;
		}
	}

	/*
	 * Apply this short delay always to ensure that we do wait tWB in
	 * any case on any machine.
	 */
	ndelay(100);

	nand_wait_ready(mtd);
}

static int jz4780_nand_dma_nand_filter(jz4780_dma_chan_t *dma_chan)
{
	/*
	 * must be special channel 1
	 */
	return dma_chan->id == 1;
}

static int jz4780_nand_request_dma(struct jz4780_nand* nand)
{
	nand_flash_if_t *nand_if;
	dma_ctrl_regs_file_t *global_regs;
	unsigned int reg_dmac;

	/*
	 * request NAND channel
	 */
	nand->dma_pipe_nand.type = JZDMA_REQ_AUTO_TXRX;
	nand->dma_pipe_nand.chan = jz4780_dma_alloc_chan(
			jz4780_nand_dma_nand_filter);
	if (!nand->dma_pipe_nand.chan)
		return -ENXIO;

	/*
	 * basic configure DMA channel
	 */
	global_regs = nand->dma_pipe_nand.chan->ctrl_regs_file;
	reg_dmac = global_regs->dmac;
	if (!(reg_dmac & BIT(1))) {
		/*
		 * enable special channel0,1
		 */
		global_regs->dmac = reg_dmac | BIT(1);

		printk(DRVNAME": enable DMA"
				" special channel<0, 1>\n");
	}

	nand_if = nand->nand_flash_if_table[0];
	nand->dma_pipe_nand.cfg.dst_addr =
			(dma_addr_t)CPHYSADDR(nand_if->cs.io_nand_dat);
	nand->dma_pipe_nand.cfg.src_addr =
			(dma_addr_t)CPHYSADDR(nand_if->cs.io_nand_dat);

	return 0;
}

static int jz4780_nand_ecc_calculate_bch(struct mtd_info *mtd,
		const uint8_t *dat, uint8_t *ecc_code)
{
	struct nand_chip *chip;
	struct jz4780_nand *nand;
	bch_request_t *req;

	chip = mtd->priv;

	if (chip->state == FL_READING)
		return 0;

	nand = mtd_to_jz4780_nand(mtd);
	req  = &nand->bch_req;

	req->raw_data = dat;
	req->type     = BCH_REQ_ENCODE;
	req->ecc_data = ecc_code;

	bch_request_submit(req);

	return 0;
}

static int jz4780_nand_ecc_correct_bch(struct mtd_info *mtd, uint8_t *dat,
		uint8_t *read_ecc, uint8_t *calc_ecc)
{
	struct jz4780_nand *nand;
	bch_request_t *req;

	nand = mtd_to_jz4780_nand(mtd);
	req  = &nand->bch_req;

	req->raw_data = dat;
	req->type     = BCH_REQ_DECODE_CORRECT;
	req->ecc_data = read_ecc;

	bch_request_submit(req);

	if (req->ret_val == BCH_RET_OK)
		return req->cnt_ecc_errors;

	return -1;
}

static int request_busy_poll(nand_flash_if_t *nand_if)
{
	int ret;

	ret = gpio_request(nand_if->busy_gpio,
				label_busy_gpio[nand_if->bank]);
	if (ret)
		return ret;

	ret = gpio_direction_input(nand_if->busy_gpio);

	return ret;
}

static nand_flash_info_t *
jz4780_nand_match_nand_chip_info(struct jz4780_nand *nand)
{
	struct mtd_info *mtd;
	struct nand_chip *chip;

	nand_flash_if_t *nand_if;
	struct jz4780_nand_platform_data *pdata;

	unsigned int nand_mfr_id = 0;
	unsigned int nand_dev_id = 0;
	int i;

	pdata = nand->pdata;
	chip = &nand->chip;
	mtd = nand->mtd;

	nand_if = nand->nand_flash_if_table[0];

	if (!chip->onfi_version) {
		/*
		 * by traditional way
		 */
		chip->select_chip(mtd, 0);
		chip->cmdfunc(mtd, NAND_CMD_RESET, -1, -1);
		chip->cmdfunc(mtd, NAND_CMD_READID, 0x00, -1);
		nand_mfr_id = chip->read_byte(mtd);
		nand_dev_id = chip->read_byte(mtd);
		chip->select_chip(mtd, -1);

		/*
		 * first match from board specific timings
		 */
		for (i = 0; i < pdata->num_nand_flash_info; i++) {
			if (nand_mfr_id ==
					pdata->nand_flash_info_table[i].nand_mfr_id &&
				nand_dev_id ==
					pdata->nand_flash_info_table[i].nand_dev_id &&
				nand_if->cs.bank_type ==
					pdata->nand_flash_info_table[i].type)

				return &pdata->nand_flash_info_table[i];
		}

		/*
		 * if got nothing form board specific timings
		 * we try to match form driver built-in timings
		 */
		for (i = 0; i < ARRAY_SIZE(builtin_nand_info_table); i++) {
			if (nand_mfr_id ==
					builtin_nand_info_table[i].nand_mfr_id &&
				nand_dev_id ==
					builtin_nand_info_table[i].nand_dev_id &&
				nand_if->cs.bank_type ==
					builtin_nand_info_table[i].type)

				return &builtin_nand_info_table[i];
		}
	} else {
		/*
		 * by ONFI way
		 */


		/*
		 * first match from board specific timings
		 */
		for (i = 0; i < pdata->num_nand_flash_info; i++) {
			if (!strncmp(chip->onfi_params.model,
					pdata->nand_flash_info_table[i].name, 20) &&
					nand_if->cs.bank_type ==
							pdata->nand_flash_info_table[i].type)
				return &pdata->nand_flash_info_table[i];
		}

		/*
		 * if got nothing form board specific timings
		 * we try to match form driver built-in timings
		 */
		for (i = 0; i < ARRAY_SIZE(builtin_nand_info_table); i++) {
			if (!strncmp(chip->onfi_params.model,
					builtin_nand_info_table[i].name, 20) &&
					nand_if->cs.bank_type ==
							builtin_nand_info_table[i].type)
				return &builtin_nand_info_table[i];
		}
	}


	if (!chip->onfi_version) {
		  printk(DRVNAME
				  ": Failed to find NAND info for devid: 0x%x\n",
						  nand_dev_id);
	} else {
		  printk(DRVNAME
				  ": Failed to find NAND info for model: %s\n",
						  chip->onfi_params.model);
	}

	return NULL;
}

static inline void jz4780_nand_cpu_read_buf(struct mtd_info *mtd,
		uint8_t *buf, int len)
{
	int i;
	struct nand_chip *chip = mtd->priv;

	for (i = 0; i < len; i++)
		buf[i] = readb(chip->IO_ADDR_R);
}

static inline void jz4780_nand_noirq_dma_read(struct mtd_info *mtd,
		uint8_t *addr, int len)
{
	struct jz4780_nand *nand = mtd_to_jz4780_nand(mtd);

	dma_chan_regs_file_t *chan_regs;

	chan_regs = nand->dma_pipe_nand.chan->chan_regs_file;

	writel((ulong)CPHYSADDR(addr), &chan_regs->dta);
	writel(nand->dma_pipe_nand.cfg.src_addr, &chan_regs->dsa);
	writel(DCM_CH1_NAND_TO_DDR | DCM_DAI |
			(7 << 8), &chan_regs->dcm);
	writel(len, &chan_regs->dtc);
	writel(nand->dma_pipe_nand.type, &chan_regs->drt);

	/*
	 * step1. INV D
	 */
	invalidate_dcache_range((ulong)addr, (ulong)addr + len);

	/*
	 * step2. start NAND transfer
	 */
	writel(BIT(31) | BIT(0), &chan_regs->dcs);

	/*
	 * step3. wait for NAND transfer done
	 */
	do {
		if (readl(&chan_regs->dcs) & DCS_TT) {
			/*
			 * step4. stop NAND transfer
			 */
			writel(0, &chan_regs->dcs);

			return;
		}
	} while (1);
}

static void jz4780_nand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	if (((uint32_t)buf & CONFIG_SYS_CACHELINE_SIZE)
			|| (len & CONFIG_SYS_CACHELINE_SIZE))
		jz4780_nand_cpu_read_buf(mtd, buf, len);
	else
		jz4780_nand_noirq_dma_read(mtd, buf, len);
}

static inline void jz4780_nand_cpu_write_buf(struct mtd_info *mtd,
		const uint8_t *buf, int len)
{
	int i;
	struct nand_chip *chip = mtd->priv;

	for (i = 0; i < len; i++)
		writeb(buf[i], chip->IO_ADDR_W);
}

static inline void jz4780_nand_noirq_dma_write(struct mtd_info *mtd,
		const uint8_t *addr, int len)
{
	struct jz4780_nand *nand = mtd_to_jz4780_nand(mtd);
	dma_chan_regs_file_t *chan_regs;

	/*
	 * step1. configure NAND channel, NAND -> KSEG1 buffer
	 */
	chan_regs = nand->dma_pipe_nand.chan->chan_regs_file;

	writel(nand->dma_pipe_nand.cfg.dst_addr, &chan_regs->dta);
	writel((ulong)CPHYSADDR(addr), &chan_regs->dsa);
	writel(DCM_CH1_DDR_TO_NAND | DCM_SAI |
			(7 << 8), &chan_regs->dcm);
	writel(len, &chan_regs->dtc);
	writel(nand->dma_pipe_nand.type, &chan_regs->drt);


	/*
	 * step2. WB D
	 */
	flush_dcache_range((ulong)addr, (ulong)addr + len);

	/*
	 * step3. start NAND transfer
	 */
	writel(BIT(31) | BIT(0), &chan_regs->dcs);
	/*
	 * step4. wait for NAND transfer done
	 */
	do {
		if (readl(&chan_regs->dcs) & DCS_TT) {
			/*
			 * step5. stop NAND transfer
			 */
			writel(0, &chan_regs->dcs);
			return;
		}
	} while (1);
}

static void jz4780_nand_write_buf(struct mtd_info *mtd,
		const uint8_t *buf, int len)
{
	if (((uint32_t)buf & CONFIG_SYS_CACHELINE_SIZE)
			|| (len & CONFIG_SYS_CACHELINE_SIZE))
		jz4780_nand_cpu_write_buf(mtd, buf, len);
	else
		jz4780_nand_noirq_dma_write(mtd, buf, len);
}

static int jz4780_nand_pre_init(struct jz4780_nand *nand)
{
	if (nand->curr_nand_flash_info->nand_pre_init)
		return nand->curr_nand_flash_info->nand_pre_init(nand);

	return 0;
}

static int jz4780_nand_onfi_set_features(struct mtd_info *mtd,
		struct nand_chip *chip, int addr, uint8_t *subfeature_param)
{
	int status;

	if (!chip->onfi_version)
		return -EINVAL;

	chip->cmdfunc(mtd, NAND_CMD_SET_FEATURES, addr, -1);
	chip->write_buf(mtd, subfeature_param, ONFI_SUBFEATURE_PARAM_LEN);
	status = chip->waitfunc(mtd, chip);
	if (status & NAND_STATUS_FAIL)
		return -EIO;
	return 0;
}

static int jz4780_nand_onfi_get_features(struct mtd_info *mtd,
		struct nand_chip *chip, int addr, uint8_t *subfeature_param)
{
	if (!chip->onfi_version)
		return -EINVAL;

	/* clear the sub feature parameters */
	memset(subfeature_param, 0, ONFI_SUBFEATURE_PARAM_LEN);

	chip->cmdfunc(mtd, NAND_CMD_GET_FEATURES, addr, -1);
	chip->read_buf(mtd, subfeature_param, ONFI_SUBFEATURE_PARAM_LEN);
	return 0;
}


/**
 * vnand_block_bad - Read bad block marker from the chip.
 *
 * @mtd: MTD device structure
 * @ofs: offset from device start
 * @getchip: 0, if the chip is already selected
 *
 * Check, if the block is bad.
 * Use first 4 bytes in oob of first 4 pages in every block as
 * bad block marker. If the number of set bits in marker is smaller
 * than 64, then the block is bad.
 */
static int vnand_block_bad(struct mtd_info *mtd, loff_t ofs, int getchip)
{
	int page, chipnr, res = 0, i = 0, j = 0;
	struct nand_chip *chip = mtd->priv;
	u16 bad;

	page = (int)(ofs >> chip->page_shift) & chip->pagemask;

	if (getchip) {
		chipnr = (int)(ofs >> chip->chip_shift);

		chip->state = FL_READING;

		/* Select the NAND device */
		chip->select_chip(mtd, chipnr);
	}

	do {
		chip->cmdfunc(mtd, NAND_CMD_READOOB, chip->badblockpos, page);

		for (j = 0; j < 4; j++) {
			bad = chip->read_byte(mtd);
			res += hweight8(bad);
		}

		ofs += mtd->writesize;
		page = (int)(ofs >> chip->page_shift) & chip->pagemask;
		i++;
	} while (i < 4);

	res = res < chip->badblockbits;

	if (getchip)
		chip->select_chip(mtd, -1);

	return res;
}

/**
 * vnand_block_markbad - mark a block bad
 * @mtd: MTD device structure
 * @ofs: offset from device start
 *
 */
static int vnand_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	struct nand_chip *chip = mtd->priv;
	uint8_t buf[4] = { 0, 0, 0, 0 };
	int block, res, ret = 0, i = 0;

	struct erase_info einfo;

	if ((ofs & (mtd->erasesize - 1)) != 0) {
		printk(DRVNAME ": Attempt to mark bad block at non "
				"block-aligned offset.\n");
		return -1;
	}

	/* Attempt erase before marking OOB */
	memset(&einfo, 0, sizeof(einfo));
	einfo.mtd = mtd;
	einfo.addr = ofs;
	einfo.len = 1 << chip->phys_erase_shift;
	nand_erase_nand(mtd, &einfo, 0);

	/* Get block number */
	block = (int)(ofs >> chip->bbt_erase_shift);
	/* Mark block bad in memory-based BBT */
	if (chip->bbt)
		chip->bbt[block >> 2] |= 0x01 << ((block & 0x03) << 1);

	/* Write bad block marker to OOB */
	struct mtd_oob_ops ops;
	loff_t wr_ofs = ofs;

	chip->state = FL_WRITING;

	ops.datbuf = NULL;
	ops.oobbuf = buf;
	ops.ooboffs = chip->badblockpos;
	ops.len = ops.ooblen = 4;

	ops.mode = MTD_OPS_PLACE_OOB;

	/* Make marks at the first 4 pages of the bad block */
	do {
		res = mtd->_write_oob(mtd, wr_ofs, &ops);
		if (!ret)
			ret = res;

		i++;
		wr_ofs += mtd->writesize;
	} while (i < 4);

	/* Make mark at the last page of the bad block for better compability */
	wr_ofs += mtd->erasesize - 5 * mtd->writesize;
	res = mtd->_write_oob(mtd, wr_ofs, &ops);
	if (!ret)
		ret = res;

	/* De-select the NAND device */
	chip->select_chip(mtd, -1);

	if (!ret)
		mtd->ecc_stats.badblocks++;

	return ret;
}

static int nand_write_spl_page(struct mtd_info *mtd,
		struct nand_chip *chip, const uint8_t *buf, int page, int ecc)
{
	int i, status, ppb;
	int eccsize, eccbytes, eccsteps;

	eccsize = SPL_ECCSIZE;
	eccbytes = bch_ecc_bits_to_bytes(SPL_BCH_BIT);
	eccsteps = mtd->writesize / eccsize;

	ppb = mtd->erasesize / mtd->writesize;

	chip->cmdfunc(mtd, NAND_CMD_SEQIN, 0x00, page);

	if (ecc == 0) {
		/* write data */
		for (i = 0; i < eccsteps; i++) {
			uint8_t *ptr;
			if (!((page & (ppb - 1)) == 0 && i == 0)) {
				gpemc_enable_pn(1);
			}
			ptr = (uint8_t *)buf + eccsize * i;
			chip->write_buf(mtd, ptr, eccsize);
		}
	} else {
		/* write ecc */
		gpemc_enable_pn(1);
		chip->write_buf(mtd, (uint8_t *)buf, eccbytes * eccsteps);
	}

	gpemc_enable_pn(0);

	chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);
	status = chip->waitfunc(mtd, chip);

	/*
	 * See if operation failed and additional status checks are
	 * available.
	 */
	if ((status & NAND_STATUS_FAIL) && (chip->errstat))
		status = chip->errstat(mtd, chip, FL_WRITING, status,
					   page);

	if (status & NAND_STATUS_FAIL)
		return -EIO;

	return 0;
}

static int jz4780_write_spl_reg(struct mtd_info *mtd, loff_t to,
		size_t len, size_t *retlen, u_char *buf)
{
	int i, j, pages, ret = 0;
	int eccsize, eccbytes, eccsteps;
	u_char *eccbuf, *datbuf;
	struct nand_chip *chip = mtd->priv;
	struct jz4780_nand *nand = mtd_to_jz4780_nand(mtd);
	bch_request_t *req;
	int start_page;
	int ecc_level_bak, blksz_bak;

	eccsize = SPL_ECCSIZE;
	eccbytes = bch_ecc_bits_to_bytes(SPL_BCH_BIT);
	if ((len != SPL_SIZE) || ((len & (mtd->writesize - 1)) != 0))
		return -EMSGSIZE;

	if ((to & (mtd->erasesize - 1)) != 0)
		return -EFAULT;

	start_page = (uint32_t)to / mtd->writesize;
	printk(DRVNAME ": spl write to page %d.\n", start_page);

	pages = len / mtd->writesize;
	eccsteps = mtd->writesize / eccsize;

	req = &nand->bch_req;
	ecc_level_bak = req->ecc_level;
	blksz_bak = req->blksz;
	req->ecc_level = SPL_BCH_BIT;
	req->blksz = eccsize;

	eccbuf = memalign(CONFIG_SYS_CACHELINE_SIZE, eccbytes * eccsteps);
	if (eccbuf == NULL)
		return -ENOMEM;

	for (i = 0; i < pages; i++) {

		datbuf = buf + mtd->writesize * i;

		for (j = 0; j < eccsteps; j++) {
			req->raw_data = datbuf + eccsize * j;
			req->type     = BCH_REQ_ENCODE;
			req->ecc_data = eccbuf + eccbytes * j;

			ret = bch_request_submit(req);
			if (ret != 0)
				goto out;
		}

		ret = nand_write_spl_page(mtd, chip, datbuf,
				start_page + 2 * i, 0);
		if (ret != 0)
			break;
		ret = nand_write_spl_page(mtd, chip, eccbuf,
				start_page + 2 * i + 1, 1);
		if (ret != 0)
			break;
	}

out:
	req->ecc_level = ecc_level_bak;
	req->blksz = blksz_bak;
	kfree(eccbuf);

	return ret;
}

static int nand_read_spl_page(struct mtd_info *mtd,
		struct nand_chip *chip, const uint8_t *buf, int page, int ecc)
{
	int i, ppb;
	int eccsize, eccbytes, eccsteps;

	eccsize = SPL_ECCSIZE;
	eccbytes = bch_ecc_bits_to_bytes(SPL_BCH_BIT);
	eccsteps = mtd->writesize / eccsize;

	ppb = mtd->erasesize / mtd->writesize;

	chip->cmdfunc(mtd, NAND_CMD_READ0, 0x00, page);

	if (ecc == 0) {
		/* read data */
		for (i = 0; i < eccsteps; i++) {
			uint8_t *ptr;
			if (!((page & (ppb - 1)) == 0 && i == 0)) {
				gpemc_enable_pn(1);
			}
			ptr = (uint8_t *)buf + eccsize * i;
			chip->read_buf(mtd, ptr, eccsize);
		}
	} else {
		/* read ecc */
		gpemc_enable_pn(1);
		chip->read_buf(mtd, (uint8_t *)buf, eccbytes * eccsteps);
	}

	gpemc_enable_pn(0);

	return 0;
}

static int jz4780_read_spl_reg(struct mtd_info *mtd, loff_t from,
			size_t len, size_t *retlen, u_char *buf)
{
	int i, j, pages, ret = 0;
	int eccsize, eccbytes, eccsteps;
	u_char *eccbuf, *datbuf;
	struct nand_chip *chip = mtd->priv;
	struct jz4780_nand *nand = mtd_to_jz4780_nand(mtd);
	bch_request_t *req;
	int start_page;
	int ecc_level_bak, blksz_bak;

	eccsize = SPL_ECCSIZE;
	eccbytes = bch_ecc_bits_to_bytes(SPL_BCH_BIT);

	if ((len != SPL_SIZE) || ((len & (mtd->writesize - 1)) != 0))
		return -EMSGSIZE;

	if ((from & (mtd->erasesize - 1)) != 0)
		return -EFAULT;

	start_page = (uint32_t)from / mtd->writesize;
	printk(DRVNAME ": spl read from page %d.\n", start_page);

	pages = len / mtd->writesize;
	eccsteps = mtd->writesize / eccsize;

	req = &nand->bch_req;
	ecc_level_bak = req->ecc_level;
	blksz_bak = req->blksz;
	req->ecc_level = SPL_BCH_BIT;
	req->blksz = eccsize;
	eccbuf = memalign(CONFIG_SYS_CACHELINE_SIZE, eccbytes * eccsteps);
	if (eccbuf == NULL)
		return -ENOMEM;

	for (i = 0; i < pages; i++) {
		datbuf = buf + mtd->writesize * i;
		ret = nand_read_spl_page(mtd, chip, datbuf,
				start_page + 2 * i, 0);
		if (ret != 0)
			break;
		ret = nand_read_spl_page(mtd, chip, eccbuf,
				start_page + 2 * i + 1, 1);
		if (ret != 0)
			break;

		for (j = 0; j < eccsteps; j++) {
			req->raw_data = datbuf + eccsize * j;
			req->type     = BCH_REQ_DECODE_CORRECT;
			req->ecc_data = eccbuf + eccbytes * j;

			ret = bch_request_submit(req);

			if (req->ret_val != BCH_RET_OK) {
				printk(DRVNAME ": spl bch uncorrect!\n");
				goto out;
			}
		}
	}

out:
	req->ecc_level = ecc_level_bak;
	req->blksz = blksz_bak;
	kfree(eccbuf);

	return ret;
}

static int jz4780_get_spl_info(struct mtd_info *mtd,
		struct otp_info *buf, size_t len)
{
	struct nand_chip *chip = mtd->priv;
	int i;
	uint8_t toggle, busw, rowcycle, pageshift;

	toggle = 0;
	busw = (chip->options & NAND_BUSWIDTH_16) ? 16 : 8;
	pageshift = chip->page_shift;

	rowcycle = 2;
	/*
	 * 3 row address cycles for the small page NAND > 32MB and the
	 * large page NAND > 128MB.
	 */
	if (mtd->writesize > 512) {
		if (chip->chipsize > (128 << 20))
			rowcycle = 3;
	} else if (chip->chipsize > (32 << 20))
		rowcycle = 3;

	for (i = 0; i < SPL_BAK_BLOCKS; i++) {
		buf->start = mtd->writesize * i;
		buf->length = SPL_SIZE;

		/* report buswidth,toggle,rowcycle,pagesize through 'locked' */
		buf->locked = busw << 24 | toggle << 16 | rowcycle << 8 |
				pageshift;
	}

	return SPL_BAK_BLOCKS * sizeof(struct otp_info);
}

static int jz4780_nand_init(int devnum)
{
	int ret = 0;
	int bank = 0;
	int i = 0, j = 0, k = 0, m = 0;
	int eccpos_start;

	struct nand_chip *chip;
	struct mtd_info *mtd;

	struct jz4780_nand *nand;
	struct jz4780_nand_platform_data *pdata = &nand_platform_data;
	nand_flash_if_t *nand_if;

	nand = kzalloc(sizeof(struct jz4780_nand), GFP_KERNEL);
	if (!nand) {
		printk(DRVNAME
			": Failed to allocate jz4780_nand.\n");
		return -ENOMEM;
	}

	nand->pdata = &nand_platform_data;

	nand->num_nand_flash_if = pdata->num_nand_flash_if;
	nand->xfer_type = pdata->xfer_type;
	nand->ecc_type = pdata->ecc_type;

	nand->mtd = &nand_info[devnum];
	nand->devnum = devnum;

	/*
	 * request GPEMC banks
	 */
	for (i = 0; i < nand->num_nand_flash_if; i++, j = i) {
		nand_if = &pdata->nand_flash_if_table[i];
		nand->nand_flash_if_table[i] = nand_if;
		bank = nand_if->bank;

		ret = gpemc_request_cs(DRVNAME, &nand_if->cs, bank);
		if (ret) {
			printk(DRVNAME
				": Failed to request GPEMC bank%d.\n", bank);
			goto err_release_cs;
		}
	}

	/*
	 * request busy GPIO interrupt
	 */
	for (i = 0; i < nand->num_nand_flash_if; i++, k = i) {
		nand_if = &pdata->nand_flash_if_table[i];

		if (nand_if->busy_gpio < 0)
			continue;

		ret = request_busy_poll(nand_if);
		if (ret) {
			printk(DRVNAME
				": Failed to request busy"
				" gpio irq for bank%d\n", bank);
			goto err_free_busy_irq;
		}
	}

	/*
	 * request WP GPIO
	 */
	for (i = 0; i < nand->num_nand_flash_if; i++, m = i) {
		nand_if = &pdata->nand_flash_if_table[i];

		if (nand_if->wp_gpio < 0)
			continue;

		bank = nand_if->bank;
		ret = gpio_request(nand_if->wp_gpio, label_wp_gpio[bank]);
		if (ret) {
			printk(DRVNAME
				": Failed to request wp GPIO:%d\n", nand_if->wp_gpio);

			goto err_free_wp_gpio;
		}

		gpio_direction_output(nand_if->wp_gpio, 0);

		/* Write protect disabled by default */
		jz4780_nand_enable_wp(nand_if, 0);
	}

	/*
	 * NAND flash devices support list override
	 */
	nand->nand_flash_table = pdata->nand_flash_table ?
		pdata->nand_flash_table : builtin_nand_flash_table;
	nand->num_nand_flash = pdata->nand_flash_table ?
		pdata->num_nand_flash :
			ARRAY_SIZE(builtin_nand_flash_table);

	/*
	 * attach to MTD subsystem
	 */
	chip              = &nand->chip;
	chip->chip_delay  = MAX_RB_DELAY_US;
	chip->cmdfunc     = jz4780_nand_command;
	chip->dev_ready   = jz4780_nand_dev_is_ready;
	chip->select_chip = jz4780_nand_select_chip;
	chip->cmd_ctrl    = jz4780_nand_cmd_ctrl;
	chip->onfi_get_features = jz4780_nand_onfi_get_features;
	chip->onfi_set_features = jz4780_nand_onfi_set_features;

	switch (nand->xfer_type) {
	case NAND_XFER_DMA_POLL:
		/*
		 * DMA transfer
		 */
		ret = jz4780_nand_request_dma(nand);
		if (ret) {
			printk(DRVNAME" Failed to request DMA channel.\n");
			goto err_free_wp_gpio;
		}

		chip->read_buf  = jz4780_nand_read_buf;
		chip->write_buf = jz4780_nand_write_buf;

		nand->use_dma = 1;

		break;

	case NAND_XFER_CPU_POLL:
		/*
		 * CPU transfer
		 */
		chip->read_buf  = jz4780_nand_cpu_read_buf;
		chip->write_buf = jz4780_nand_cpu_write_buf;

		break;

	default:
		printk(DRVNAME " Unsupport transfer type.\n");
		BUG();

		break;
	}

	mtd              = nand->mtd;
	mtd->priv        = chip;

	/*
	 * nand_base handle subpage write by fill space
	 * where are outside of the subpage with 0xff,
	 * that make things totally wrong, so disable it.
	 */
	chip->options |= NAND_NO_SUBPAGE_WRITE;

	/*
	 * Detect NAND flash chips
	 */

	/* step1. relax bank timings to scan */
	for (bank = 0; bank < nand->num_nand_flash_if; bank++) {
		nand_if = nand->nand_flash_if_table[bank];

		gpemc_relax_bank_timing(&nand_if->cs);
	}

	if (nand_scan_ident(mtd, nand->num_nand_flash_if,
			nand->nand_flash_table)) {

		ret = -ENXIO;
		printk(DRVNAME" Failed to detect NAND flash.\n");
		goto err_dma_release_channel;
	}

	/*
	 * post configure bank timing by detected NAND device
	 */

	/* step1. match NAND chip information */
	nand->curr_nand_flash_info = jz4780_nand_match_nand_chip_info(nand);
	if (!nand->curr_nand_flash_info) {
		ret = -ENODEV;
		goto err_dma_release_channel;
	}

	/*
	 * step2. preinitialize NAND flash
	 */
	ret = jz4780_nand_pre_init(nand);
	if (ret) {
		printk(DRVNAME": Failed to"
				" preinitialize NAND chip.\n");
		goto err_dma_release_channel;
	}

	/* step3. replace NAND command function with large page version */
	if (mtd->writesize > 512)
		chip->cmdfunc = jz4780_nand_command_lp;

	/* step4. configure bank timings */
	switch (nand->curr_nand_flash_info->type) {
	case BANK_TYPE_NAND:
		for (bank = 0; bank < nand->num_nand_flash_if; bank++) {
			nand_if = nand->nand_flash_if_table[bank];

			gpemc_fill_timing_from_nand(&nand_if->cs,
				&nand->curr_nand_flash_info->
				nand_timing.common_nand_timing);

			ret = gpemc_config_bank_timing(&nand_if->cs);
			if (ret) {
				printk(DRVNAME
					": Failed to configure timings for bank%d\n"
						, nand_if->bank);
				goto err_dma_release_channel;
			}
		}

		break;

	case BANK_TYPE_TOGGLE:
		for (bank = 0; bank < nand->num_nand_flash_if; bank++) {
			nand_if = nand->nand_flash_if_table[bank];

			gpemc_fill_timing_from_toggle(&nand_if->cs,
				&nand->curr_nand_flash_info->
				nand_timing.toggle_nand_timing);

			ret = gpemc_config_bank_timing(&nand_if->cs);
			if (ret) {
				printk(DRVNAME
					": Failed to configure timings for bank%d\n"
						, nand_if->bank);
				goto err_dma_release_channel;
			}
		}

		break;

	default:
		printk(DRVNAME": Unsupported NAND type.\n");
		BUG();

		break;
	}

	/* VNAND configuration override */
	if (pdata->vnand.is_vnand) {
		chip->badblockpos = 0;
		chip->badblockbits = 64;
		chip->block_bad = vnand_block_bad;
		chip->block_markbad = vnand_block_markbad;
		/* With no memory bbt, chip->block_bad() is used. */
		chip->options |= NAND_SKIP_BBTSCAN;
	}

	/*
	 * initialize ECC control
	 */

	/* step1. configure ECC step */
	switch (nand->ecc_type) {
	case NAND_ECC_TYPE_SW:
		chip->ecc.mode  = NAND_ECC_SOFT_BCH;
		chip->ecc.size  =
			nand->curr_nand_flash_info->ecc_step.data_size;
		chip->ecc.bytes = (fls(8 * chip->ecc.size) *
			(nand->curr_nand_flash_info->ecc_step.ecc_bits) + 7) / 8;

		break;

	case NAND_ECC_TYPE_HW:
		nand->bch_req.ecc_level =
				nand->curr_nand_flash_info->ecc_step.ecc_bits;
		nand->bch_req.blksz     =
				nand->curr_nand_flash_info->ecc_step.data_size;

		nand->bch_req.errrept_data = kzalloc(MAX_ERRREPT_DATA_SIZE,
				GFP_KERNEL);
		if (!nand->bch_req.errrept_data) {
			printk(DRVNAME
				": Failed to allocate ECC errrept_data buffer\n");
			ret = -ENOMEM;
			goto err_dma_release_channel;
		}

		chip->ecc.mode      = NAND_ECC_HW;
		chip->ecc.calculate = jz4780_nand_ecc_calculate_bch;
		chip->ecc.correct   = jz4780_nand_ecc_correct_bch;
		chip->ecc.hwctl     = jz4780_nand_ecc_hwctl;
		chip->ecc.size  =
			nand->curr_nand_flash_info->ecc_step.data_size;
		chip->ecc.bytes = bch_ecc_bits_to_bytes(
			nand->curr_nand_flash_info->ecc_step.ecc_bits);
		chip->ecc.strength = nand->bch_req.ecc_level;

		break;

	default :
		printk(DRVNAME": Unsupported ECC type.\n");
		BUG();

		break;
	}

	/* step2. generate ECC layout */

	/*
	 * eccbytes = eccsteps * eccbytes_prestep;
	 */
	nand->ecclayout.eccbytes =
		mtd->writesize / chip->ecc.size * chip->ecc.bytes;

	if (mtd->oobsize < (nand->ecclayout.eccbytes +
			chip->badblockpos + 2)) {
		printk(DRVNAME": ECC codes are out of OOB area.\n");
		BUG();
	}

	/*
	 * ECC codes are right aligned
	 * start position = oobsize - eccbytes
	 */
	if (pdata->vnand.is_vnand)
		eccpos_start = pdata->vnand.eccpos;
	else
		eccpos_start = mtd->oobsize - nand->ecclayout.eccbytes;

	for (bank = 0; bank < nand->ecclayout.eccbytes; bank++)
		nand->ecclayout.eccpos[bank] = eccpos_start + bank;

	nand->ecclayout.oobfree->offset = chip->badblockpos + 2;
	nand->ecclayout.oobfree->length =
		mtd->oobsize - (nand->ecclayout.eccbytes
			+ chip->badblockpos + 2);

	chip->ecc.layout = &nand->ecclayout;

	/*
	 * second phase NAND scan
	 */
	if (nand_scan_tail(mtd)) {
		ret = -ENXIO;
		goto err_free_ecc;
	}

	mtd->_write_user_prot_reg = jz4780_write_spl_reg;
	mtd->_read_user_prot_reg = jz4780_read_spl_reg;
	mtd->_get_user_prot_info = jz4780_get_spl_info;

	printk(DRVNAME
		": Successfully registered JZ4780 SoC NAND controller driver.\n");

	return 0;

err_free_ecc:
	if (pdata->ecc_type == NAND_ECC_TYPE_HW)
		kfree(nand->bch_req.errrept_data);

err_dma_release_channel:
	if (nand->xfer_type == NAND_XFER_DMA_POLL)
		jz4780_dma_release_chan(nand->dma_pipe_nand.chan);

err_free_wp_gpio:
	for (bank = 0; bank < m; bank++) {
		nand_if = &pdata->nand_flash_if_table[bank];

		if (nand_if->wp_gpio < 0)
			continue;

		gpio_free(nand_if->wp_gpio);
	}

err_free_busy_irq:
	for (bank = 0; bank < k; bank++) {
		nand_if = &pdata->nand_flash_if_table[bank];

		if (nand_if->busy_gpio < 0)
			continue;

		gpio_free(nand_if->busy_gpio);
	}

err_release_cs:
	for (bank = 0; bank < j; bank++) {
		nand_if = &pdata->nand_flash_if_table[bank];

		gpemc_release_cs(&nand_if->cs);
	}

	kfree(nand);

	return ret;
}

/*
 * driver entry point
 */
int jz4780_board_nand_init(void)
{
	int ret;

	if (CONFIG_SYS_MAX_NAND_DEVICE > 1) {
		printk(DRVNAME": can not support"
				" CONFIG_SYS_MAX_NAND_DEVICE > 1");
		BUG();
	}

	ret = jz4780_nand_init(0);

	if (!ret)
		nand_register(0);

	return ret;
}

static int
jz4780_nand_debugfs_show(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct jz4780_nand *nand = mtd_to_jz4780_nand(&nand_info[0]);
	nand_flash_if_t *nand_if;
	nand_flash_info_t *nand_info = nand->curr_nand_flash_info;
	int i, j;

	printk("Attached banks:\n");
	for (i = 0; i < nand->num_nand_flash_if; i++) {
		nand_if = nand->nand_flash_if_table[i];

		printk("bank%d\n", nand_if->bank);
	}

	if (nand->curr_nand_flash_if != -1) {
		nand_if = nand->nand_flash_if_table[nand->curr_nand_flash_if];
		printk("selected: bank%d\n",
				nand_if->bank);
	} else {
		printk("selected: none\n");
	}

	if (nand_info) {
		printk("\n");
		printk("Attached NAND flash:\n");
		printk("Chip name: %s\n", nand_info->name);
		if (nand->chip.onfi_version) {
			printk("ONFI: v%d\n", nand->chip.onfi_version);
			printk("Timing mode: %d\n",
					nand_info->onfi_special.timing_mode);
			printk("Chip mfrid: 0x%x(%s)\n", nand_info->nand_mfr_id,
					nand->chip.onfi_params.manufacturer);
		} else {
			printk("ONFI: unsupported\n");
			/* Try to identify manufacturer */
			for (i = 0; nand_manuf_ids[i].id != 0x0; i++) {
				if (nand_manuf_ids[i].id == nand_info->nand_mfr_id)
					break;
			}
			printk("Chip mfrid: 0x%x(%s)\n", nand_info->nand_mfr_id,
					nand_manuf_ids[i].name);
		}

		printk("Chip devid: 0x%x\n", nand_info->nand_dev_id);
		printk("Chip size: %dMB\n", (int)(nand->chip.chipsize >> 20));
		printk("Erase size: %ubyte\n", nand->mtd->erasesize);
		printk("Write size: %dbyte\n", nand->mtd->writesize);
		printk("OOB size: %dbyte\n", nand->mtd->oobsize);

		printk("\n");
		printk("Data path:\n");
		printk("Use DMA: %d\n", nand->use_dma);

		printk("\n");
		printk("NAND flash output driver strength: ");
		switch (nand_info->output_strength) {
		case NAND_OUTPUT_NORMAL_DRIVER:
			printk("Normal driver\n");
			break;

		case NAND_OUTPUT_UNDER_DRIVER1:
			printk("Under driver(1)\n");
			break;
		case NAND_OUTPUT_UNDER_DRIVER2:
			printk("Under driver(2)\n");
			break;

		case NAND_OUTPUT_OVER_DRIVER1:
			printk("Over driver(1)\n");
			break;
		case NAND_OUTPUT_OVER_DRIVER2:
			printk("Over driver(2)\n");
			break;
		case CAN_NOT_ADJUST_OUTPUT_STRENGTH:
			printk("unsupport\n");
			break;
		}
		printk("NAND flash R/B# pull-down driver strength: ");
		switch (nand_info->rb_down_strength) {
		case NAND_RB_DOWN_FULL_DRIVER:
			printk("full\n");
			break;

		case NAND_RB_DOWN_THREE_QUARTER_DRIVER:
			printk("3/4 full\n");
			break;

		case NAND_RB_DOWN_ONE_HALF_DRIVER:
			printk("1/2 full\n");
			break;

		case NAND_RB_DOWN_ONE_QUARTER_DRIVER:
			printk("1/4 full\n");
			break;

		case CAN_NOT_ADJUST_RB_DOWN_STRENGTH:
			printk("unsupport\n");
			break;
		}

		printk("\n");
		printk("Attached NAND flash ECC:\n");
		printk("ECC type: %s\n", nand->ecc_type == NAND_ECC_TYPE_HW ?
				"HW-BCH" : "SW-BCH");

		printk("ECC size: %dbyte\n", nand->chip.ecc.size);
		printk("ECC bits: %d\n", nand_info->ecc_step.ecc_bits);
		printk("ECC bytes: %d\n", nand->chip.ecc.bytes);
		printk("ECC steps: %d\n", nand->chip.ecc.steps);

		printk("\n");
		printk("ECC layout:\n");
		printk("ecclayout.eccbytes: %d\n", nand->ecclayout.eccbytes);
		printk("ecclayout.eccpos:\n");
		for (i = 0; i < nand->chip.ecc.steps; i++) {
			printk("ecc step: %d\n", i + 1);
			for (j = 0; j < nand->chip.ecc.bytes - 1; j++) {
				printk("%d, ",
						nand->ecclayout.eccpos[i * nand->chip.ecc.bytes + j]);

				if ((j + 1) % 10 == 0)
					printk("\n");
			}

			printk("%d\n",
					nand->ecclayout.eccpos[i * nand->chip.ecc.bytes + j]);
		}

		printk("ecclayout.oobavail: %d\n", nand->ecclayout.oobavail);
		printk("ecclayout.oobfree:\n");
		for (i = 0; nand->ecclayout.oobfree[i].length &&
					i < ARRAY_SIZE(nand->ecclayout.oobfree); i++) {
			printk("oobfree[%d]:\n", i);
			printk("length: %u\n", nand->ecclayout.oobfree[i].length);
			printk("offset: %u\n", nand->ecclayout.oobfree[i].offset);
		}
	}

	return 0;
}

U_BOOT_CMD(
	nandinfo, 1, 1, jz4780_nand_debugfs_show,
	"show NAND info",
	"cmd: nandinfo"
);

/*
 * MODULE_LICENSE("GPL");
 * MODULE_AUTHOR("Fighter Sun <wanmyqawdr@126.com>");
 * MODULE_DESCRIPTION("NAND controller driver for JZ4780 SoC");
 * MODULE_ALIAS("platform:"DRVNAME);
 */
