/*
 * Ingenic MMC/SD Controller driver
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co., Ltd.
 * Authors: Kage Shen <kkshen@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 3 of the License, or (at your option) any later version.
 */
#include <config.h>
#include <common.h>
#include <mmc.h>
#include <part.h>
#include <malloc.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <asm/arch/mmc.h>
#include <asm/arch/cpm.h>
#if defined(CONFIG_JZ4780)
#include <asm/arch/jz4780_gpio.h>
#elif defined(CONFIG_JZ4775)
#include <asm/arch/jz4775_gpio.h>
#elif defined(CONFIG_M200)
#include <asm/arch/m200_gpio.h>
#endif
/*
#include <asm/jz4780.h>
#undef REG_CPM_MSCCDR
#undef MSC0_BASE

#define __msc_gpio_func_init()	__gpio_as_msc1_pe_4bit()
#define REG_CPM_MSCCDR			REG32(CPM_MSCCDR1)
#define	MSC0_BASE				MSC1_BASE

#define MMC_IRQ_MASK()					\
	do {								\
		REG_MSC_IMASK = 0xffff;			\
		REG_MSC_IREG = 0xffff;			\
	} while (0)
 */

#define	MSC0_REG_BASE		0xB3450000
#define	MSC1_REG_BASE		0xB3460000
#define	MSC2_REG_BASE		0xB3470000

/* MSC Clock and Control Register (MSC_CTRL) */
#define CTRL_SEND_CCSD          (1 << 15) /*send command completion signal disable to ceata */
#define CTRL_SEND_AS_CCSD       (1 << 14) /*send internally generated stop after sending ccsd */
#define CTRL_EXIT_MULTIPLE      (1 << 7)
#define CTRL_EXIT_TRANSFER      (1 << 6)
#define CTRL_START_READWAIT     (1 << 5)
#define CTRL_STOP_READWAIT      (1 << 4)
#define CTRL_RESET              (1 << 3)
#define CTRL_START_OP           (1 << 2)
#define CTRL_CLOCK_SHF          0
#define CTRL_CLOCK_MASK         (0x3 << CTRL_CLOCK_SHF)
#define CTRL_CLOCK_STOP         (0x1 << CTRL_CLOCK_SHF) /* Stop MMC/SD clock */
#define CTRL_CLOCK_START        (0x2 << CTRL_CLOCK_SHF) /* Start MMC/SD clock */

/* MSC Control 2 Register (MSC_CTRL2) */
#define	CTRL2_PIP_SHF           24
#define	CTRL2_PIP_MASK          (0x1f << CTRL2_PIP_SHF)
#define	CTRL2_RST_EN            (1 << 23)
#define	CTRL2_STPRM             (1 << 4)
#define	CTRL2_SVC               (1 << 3)
#define	CTRL2_SMS_SHF           0
#define	CTRL2_SMS_MASK          (0x7 << CTRL2_SMS_SHF)
#define	CTRL2_SMS_DEFSPD        (0x0 << CTRL2_SMS_SHF)
#define	CTRL2_SMS_HISPD         (0x1 << CTRL2_SMS_SHF)
#define	CTRL2_SMS_SDR12         (0x2 << CTRL2_SMS_SHF)
#define	CTRL2_SMS_SDR25         (0x3 << CTRL2_SMS_SHF)
#define	CTRL2_SMS_SDR50         (0x4 << CTRL2_SMS_SHF)

/* MSC Status Register (MSC_STAT) */
#define STAT_AUTO_CMD12_DONE        (1 << 31)
#define STAT_AUTO_CMD23_DONE        (1 << 30)
#define STAT_SVS                (1 << 29)
#define STAT_PIN_LEVEL_SHF       24
#define STAT_PIN_LEVEL_MASK     (0x1f << STAT_PIN_LEVEL_SHF)
#define STAT_BCE          (1 << 20)
#define STAT_BDE          (1 << 19)
#define STAT_BAE          (1 << 18)
#define STAT_BAR          (1 << 17)
#define STAT_DMAEND       (1 << 16)
#define STAT_IS_RESETTING       (1 << 15)
#define STAT_SDIO_INT_ACTIVE    (1 << 14)
#define STAT_PRG_DONE           (1 << 13)
#define STAT_DATA_TRAN_DONE     (1 << 12)
#define STAT_END_CMD_RES        (1 << 11)
#define STAT_DATA_FIFO_AFULL    (1 << 10)
#define STAT_IS_READWAIT        (1 << 9)
#define STAT_CLK_EN             (1 << 8)
#define STAT_DATA_FIFO_FULL     (1 << 7)
#define STAT_DATA_FIFO_EMPTY    (1 << 6)
#define STAT_CRC_RES_ERR        (1 << 5)
#define STAT_CRC_READ_ERROR     (1 << 4)
#define STAT_CRC_WRITE_ERROR_SHF     2
#define STAT_CRC_WRITE_ERROR_MASK   (0x3 << STAT_CRC_WRITE_ERROR_SHF)
#define STAT_CRC_WRITE_ERROR_NO     (0 << STAT_CRC_WRITE_ERROR_SHF)
#define STAT_CRC_WRITE_ERROR        (1 << STAT_CRC_WRITE_ERROR_SHF)
#define STAT_CRC_WRITE_ERROR_NOSTS  (2 << STAT_CRC_WRITE_ERROR_SHF)
#define STAT_TIME_OUT_RES         (1 << 1)
#define STAT_TIME_OUT_READ        (1 << 0)
#define ERROR_STAT          0x3f
#define ERROR_TIMEOUT       0x3
#define ERROR_CRC           0x3c

/* MSC Bus Clock Control Register (MSC_CLKRT) */
#define	CLKRT_CLK_RATE_SHF          0
#define	CLKRT_CLK_RATE_MASK         (0x7 << CLKRT_CLK_RATE_SHF)
#define CLKRT_CLK_RATE_DIV_1        (0x0 << CLKRT_CLK_RATE_SHF) /* CLK_SRC */
#define CLKRT_CLK_RATE_DIV_2        (0x1 << CLKRT_CLK_RATE_SHF) /* 1/2 of CLK_SRC */
#define CLKRT_CLK_RATE_DIV_4        (0x2 << CLKRT_CLK_RATE_SHF) /* 1/4 of CLK_SRC */
#define CLKRT_CLK_RATE_DIV_8        (0x3 << CLKRT_CLK_RATE_SHF) /* 1/8 of CLK_SRC */
#define CLKRT_CLK_RATE_DIV_16       (0x4 << CLKRT_CLK_RATE_SHF) /* 1/16 of CLK_SRC */
#define CLKRT_CLK_RATE_DIV_32       (0x5 << CLKRT_CLK_RATE_SHF) /* 1/32 of CLK_SRC */
#define CLKRT_CLK_RATE_DIV_64       (0x6 << CLKRT_CLK_RATE_SHF) /* 1/64 of CLK_SRC */
#define CLKRT_CLK_RATE_DIV_128      (0x7 << CLKRT_CLK_RATE_SHF) /* 1/128 of CLK_SRC */

/* MSC Command Sequence Control Register (MSC_CMDAT) */
#define	CMDAT_CCS_EXPECTED      (1 << 31) /* interrupts are enabled in ce-ata */
#define	CMDAT_READ_CEATA        (1 << 30)
#define	CMDAT_DIS_BOOT          (1 << 27)
#define	CMDAT_ENB_BOOT          (1 << 26)
#define	CMDAT_EXP_BOOT_ACK      (1 << 25)
#define	CMDAT_BOOT_MODE         (1 << 24)
#define	CMDAT_AUTO_CMD23        (1 << 18)
#define	CMDAT_SDIO_PRDT         (1 << 17) /* exact 2 cycle */
#define	CMDAT_AUTO_CMD12        (1 << 16)
#define	CMDAT_RTRG_SHF          14
#define CMDAT_RTRG_EQUALT_16        (0x0 << CMDAT_RTRG_SHF) /*reset value*/
#define CMDAT_RTRG_EQUALT_32        (0x1 << CMDAT_RTRG_SHF)
#define CMDAT_RTRG_EQUALT_64        (0x2 << CMDAT_RTRG_SHF)
#define CMDAT_RTRG_EQUALT_96        (0x3 << CMDAT_RTRG_SHF)
#define	CMDAT_TTRG_SHF           12
#define CMDAT_TTRG_LESS_16      (0x0 << CMDAT_TTRG_SHF) /*reset value*/
#define CMDAT_TTRG_LESS_32      (0x1 << CMDAT_TTRG_SHF)
#define CMDAT_TTRG_LESS_64      (0x2 << CMDAT_TTRG_SHF)
#define CMDAT_TTRG_LESS_96      (0x3 << CMDAT_TTRG_SHF)
#define	CMDAT_IO_ABORT          (1 << 11)
#define	CMDAT_BUS_WIDTH_SHF     9
#define	CMDAT_BUS_WIDTH_MASK        (0x3 << CMDAT_BUS_WIDTH_SHF)
#define CMDAT_BUS_WIDTH_1BIT        (0x0 << CMDAT_BUS_WIDTH_SHF) /* 1-bit data bus */
#define CMDAT_BUS_WIDTH_4BIT        (0x2 << CMDAT_BUS_WIDTH_SHF) /* 4-bit data bus */
#define CMDAT_BUS_WIDTH_8BIT         (0x3 << CMDAT_BUS_WIDTH_SHF) /* 8-bit data bus */
#define	CMDAT_INIT              (1 << 7)
#define	CMDAT_BUSY              (1 << 6)
#define	CMDAT_STREAM_BLOCK      (1 << 5)
#define	CMDAT_WRITE_READ        (1 << 4)
#define	CMDAT_DATA_EN           (1 << 3)
#define	CMDAT_RESPONSE_SHF      0
#define	CMDAT_RESPONSE_MASK     (0x7 << CMDAT_RESPONSE_SHF)
#define CMDAT_RESPONSE_NONE     (0x0 << CMDAT_RESPONSE_SHF) /* No response */
#define CMDAT_RESPONSE_R1       (0x1 << CMDAT_RESPONSE_SHF) /* Format R1 and R1b */
#define CMDAT_RESPONSE_R2       (0x2 << CMDAT_RESPONSE_SHF) /* Format R2 */
#define CMDAT_RESPONSE_R3       (0x3 << CMDAT_RESPONSE_SHF) /* Format R3 */
#define CMDAT_RESPONSE_R4       (0x4 << CMDAT_RESPONSE_SHF) /* Format R4 */
#define CMDAT_RESPONSE_R5       (0x5 << CMDAT_RESPONSE_SHF) /* Format R5 */
#define CMDAT_RESPONSE_R6       (0x6 << CMDAT_RESPONSE_SHF) /* Format R6 */
#define CMDAT_RESRONSE_R7       (0x7 << CMDAT_RESPONSE_SHF) /* Format R7 */

/* MSC Interrupts Mask Register (MSC_IMASK) */
#define	IMASK_AUTO_CMD23_DONE       (1 << 30)
#define	IMASK_SVS                   (1 << 29)
#define	IMASK_PIN_LEVEL_SHF         24
#define	IMASK_PIN_LEVEL_MASK        (0x1f << IMASK_PIN_LEVEL_SHF)
#define	IMASK_BCE         (1 << 20)
#define	IMASK_BDE         (1 << 19)
#define	IMASK_BAE         (1 << 18)
#define	IMASK_BAR         (1 << 17)
#define	IMASK_DMAEND            (1 << 16)
#define	IMASK_AUTO_CMD12_DONE   (1 << 15)
#define	IMASK_DATA_FIFO_FULL    (1 << 14)
#define	IMASK_DATA_FIFO_EMP     (1 << 13)
#define	IMASK_CRC_RES_ERR       (1 << 12)
#define	IMASK_CRC_READ_ERR      (1 << 11)
#define	IMASK_CRC_WRITE_ERR     (1 << 10)
#define	IMASK_TIME_OUT_RES      (1 << 9)
#define	IMASK_TIME_OUT_READ     (1 << 8)
#define	IMASK_SDIO              (1 << 7)
#define	IMASK_TXFIFO_WR_REQ     (1 << 6)
#define	IMASK_RXFIFO_RD_REQ     (1 << 5)
#define	IMASK_END_CMD_RES       (1 << 2)
#define	IMASK_PRG_DONE          (1 << 1)
#define	IMASK_DATA_TRAN_DONE    (1 << 0)

/* MSC Interrupts Status Register (MSC_IREG) */
#define	IFLG_AUTO_CMD23_DONE    (1 << 30)
#define	IFLG_SVS                (1 << 29)
#define	IFLG_PIN_LEVEL_SHF      24
#define	IFLG_PIN_LEVEL_MASK     (0x1f << IFLG_PIN_LEVEL_SHF)
#define	IFLG_BCE            (1 << 20)
#define	IFLG_BDE            (1 << 19)
#define	IFLG_BAE            (1 << 18)
#define	IFLG_BAR            (1 << 17)
#define	IFLG_DMAEND         (1 << 16)
#define	IFLG_AUTO_CMD12_DONE    (1 << 15)
#define	IFLG_DATA_FIFO_FULL     (1 << 14)
#define	IFLG_DATA_FIFO_EMP      (1 << 13)
#define	IFLG_CRC_RES_ERR        (1 << 12)
#define	IFLG_CRC_READ_ERR       (1 << 11)
#define	IFLG_CRC_WRITE_ERR      (1 << 10)
#define	IFLG_TIMEOUT_RES        (1 << 9)
#define	IFLG_TIMEOUT_READ       (1 << 8)
#define	IFLG_SDIO               (1 << 7)
#define	IFLG_TXFIFO_WR_REQ      (1 << 6)
#define	IFLG_RXFIFO_RD_REQ      (1 << 5)
#define	IFLG_END_CMD_RES        (1 << 2)
#define	IFLG_PRG_DONE           (1 << 1)
#define	IFLG_DATA_TRAN_DONE     (1 << 0)

/* MSC Low Power Mode Register (MSC_LPM) */
#define	LPM_DRV_SEL_SHF         30
#define	LPM_DRV_SEL_MASK        (0x3 << LPM_DRV_SEL_SHF)
#define	LPM_SMP_SEL             (1 << 29)
#define	LPM_LPM                 (1 << 0)

/* MSC DMA Control Register (MSC_DMAC) */
#define	DMAC_MODE_SEL       (1 << 7)
#define	DMAC_AOFST_SHF      5
#define	DMAC_AOFST_MASK     (0x3 << DMAC_AOFST_SHF)
#define	DMAC_AOFST_0        (0 << DMAC_AOFST_SHF)
#define	DMAC_AOFST_1        (1 << DMAC_AOFST_SHF)
#define	DMAC_AOFST_2        (2 << DMAC_AOFST_SHF)
#define	DMAC_AOFST_3        (3 << DMAC_AOFST_SHF)
#define	DMAC_ALIGNEN        (1 << 4)
#define	DMAC_INCR_SHF       2
#define	DMAC_INCR_MASK      (0x3 << DMAC_INCR_SHF)
#define	DMAC_INCR_16        (0 << DMAC_INCR_SHF)
#define	DMAC_INCR_32        (1 << DMAC_INCR_SHF)
#define	DMAC_INCR_64        (2 << DMAC_INCR_SHF)
#define	DMAC_DMASEL         (1 << 1)
#define	DMAC_DMAEN          (1 << 0)

#define PIO_THRESHOLD       64	/* use pio mode if data length < PIO_THRESHOLD */

struct sdma_desc {
	volatile u32 nda;
	volatile u32 da;
	volatile u32 len;
	volatile u32 dcmd;
};

struct sdma_desc adma[4];
static struct mmc hsmmc_dev[2];
static struct m200_mmc_host mmc_host[3];

static inline u32 read_mmc_res(struct mmc *mmc, u32 flag)
{
	struct m200_mmc_host *host = (struct m200_mmc_host *)mmc->priv;
	struct hsmmc *mmc_base = host->regs;
	u32 resp = mmc_base->res;
	if(flag)
		resp = (resp << 16) | mmc_base->res;

	return resp;
}

static inline int wait_set_status(struct hsmmc *mmc_base, u32 t)
{
	u32 timeout = 0x6fffff;

	while(timeout-- && !(readl(&mmc_base->stat) & t)) {
		if(timeout < 0xF) {
			return 1;
		}
	}
	return 0;
}

static inline int wait_clear_status(struct hsmmc *mmc_base, u32 t)
{
	u32 timeout = 0x6fffff;

	while(timeout-- && (readl(&mmc_base->stat) & t)) {
		if(timeout < 0xF) {
			return 1;
		}
	}
	return 0;
}

static inline u32 mmc_prepare_data(struct mmc *mmc, struct mmc_cmd *cmd,
		struct mmc_data *data)
{
	u32 cmdat;
	struct m200_mmc_host *host = (struct m200_mmc_host *)mmc->priv;
	struct hsmmc *mmc_base = host->regs;

	writel(0xffffff, &mmc_base->rdto);
	writel(data->blocks, &mmc_base->nob);
	writel(data->blocksize, &mmc_base->blklen);

	cmdat = CMDAT_DATA_EN;
	writel(CMDAT_DATA_EN, &mmc_base->cmdat);

	if (data->flags & MMC_DATA_WRITE)
		cmdat |= CMDAT_WRITE_READ;
	else if (data->flags & MMC_DATA_READ)
		cmdat &= ~CMDAT_WRITE_READ;

	return cmdat;
}

static int data_trans_done(struct mmc *mmc, struct mmc_data *data)
{
	struct m200_mmc_host *host = (struct m200_mmc_host *)mmc->priv;
	struct hsmmc *mmc_base = host->regs;

	if (data->flags & MMC_DATA_WRITE) {
		if(wait_set_status(mmc_base, STAT_PRG_DONE)) {
			return -EIO;
		}
		writel(IFLG_PRG_DONE, &mmc_base->ireg);
	} else {
		if(wait_set_status(mmc_base, STAT_DATA_TRAN_DONE)) {
			return -EIO;
		}
		writel(IFLG_DATA_TRAN_DONE, &mmc_base->ireg);
	}

	return 0;
}

static int start_dma_trans(struct mmc *mmc, u32 *buf, u32 size)
{
	struct m200_mmc_host *host = (struct m200_mmc_host *)mmc->priv;
	struct hsmmc *mmc_base = host->regs;
	u32 dmac = 5;

	struct sdma_desc *pdma;

	pdma = (struct sdma_desc *)(((u32)adma));

	/* If the address is not in accordance with the 4BYTEs alignment */
	if (((u32)buf & 0x3) || (size & 0x3)) {
		dmac |= DMAC_ALIGNEN;
		if ((u32)buf & 0x3)
			dmac |= ((u32)buf % 4) << DMAC_AOFST_SHF;
	}

	pdma->nda = 0;
	pdma->da = (u32)virt_to_phys(buf);
	pdma->len = size;
	pdma->dcmd = 2;

	flush_dcache_range((u32)pdma, (u32)pdma + 32);

	writel((u32)virt_to_phys(pdma), &mmc_base->dmanda);
	writel(dmac, &mmc_base->dmac);

	if(wait_set_status(mmc_base, STAT_DMAEND)) {
		printf("Unable to open the sDMA transfer\n");
		return -EIO;
	}

	while((readl(&mmc_base->stat) & STAT_DATA_TRAN_DONE) == 0) {
		u32 tmp = readl(&mmc_base->dmalen);
		mdelay(50);
		if(tmp && (tmp == readl(&mmc_base->dmalen))) {
			printf("sDMA transfer error\n");
			return -EIO;
		}
	}

	invalidate_dcache_range((u32)buf, (u32)buf + size);

	return 0;
}

static int mmc_read_data(struct mmc *mmc, u32 *buf, u32 size)
{
	struct m200_mmc_host *host = (struct m200_mmc_host *)mmc->priv;
	struct hsmmc *mmc_base = host->regs;
	u32 i;

	if(size < PIO_THRESHOLD) {
		for (i = 0; i < size / 4; i++) {
			if(wait_clear_status(mmc_base, STAT_DATA_FIFO_EMPTY)) {
				printf("%s: timedout waiting for read data!\n", __func__);
				return -EIO;
			}
			*(buf + i)= readl(&mmc_base->rxfifo);
		}
	} else {
		start_dma_trans(mmc, buf, size);
	}

	return 0;
}

static int mmc_write_data(struct mmc *mmc, u32 *buf, u32 size)
{
	struct m200_mmc_host *host = (struct m200_mmc_host *)mmc->priv;
	struct hsmmc *mmc_base = host->regs;
	u32 i;

	for (i = 0; i < size / 4; i++) {
		if(wait_clear_status(mmc_base, STAT_DATA_FIFO_FULL)) {
			printf("%s: timedout waiting for write data!\n", __func__);
			return -EIO;
		}
		writel(*(buf + i), &mmc_base->txfifo);
	}
	return 0;
}

static int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
			struct mmc_data *data)
{
	struct m200_mmc_host *host = (struct m200_mmc_host *)mmc->priv;
	struct hsmmc *mmc_base = host->regs;
	static int auto_cmd12 = 0;
	int ret = 0;
	u32 cmdat = 0;
	u32 i, res;

	/* configue bus width */
	switch (mmc->bus_width) {
	case 8:
		cmdat |= CMDAT_BUS_WIDTH_8BIT;
		break;
	case 4:
		cmdat |= CMDAT_BUS_WIDTH_4BIT;
		break;
	case 1:
		cmdat |= CMDAT_BUS_WIDTH_1BIT;
		break;
	default:
		printf("Can't support %d bus width\n", mmc->bus_width);
		return -EINVAL;
	}

	/* Inform the controller about response type */
	switch (cmd->resp_type) {
	case MMC_RSP_R1:
	case MMC_RSP_R1b:
		cmdat |= CMDAT_RESPONSE_R1;
		break;
	case MMC_RSP_R2:
		cmdat |= CMDAT_RESPONSE_R2;
		break;
	case MMC_RSP_R3:
		cmdat |= CMDAT_RESPONSE_R3;
		break;
	default:
		/* r1 = r5,r6,r7 */
		cmdat |= CMDAT_RESPONSE_R1;
		break;
	}

	/* The card can send a "busy" response */
	if (cmd->resp_type & MMC_RSP_BUSY)
		cmdat |= CMDAT_BUSY;

	if(data)
		cmdat |= mmc_prepare_data(mmc, cmd, data);

	/* auto send cmd12 when multiple block write and read */
	if (cmd->cmdidx == MMC_CMD_WRITE_MULTIPLE_BLOCK ||
			cmd->cmdidx == MMC_CMD_READ_MULTIPLE_BLOCK) {
		cmdat |= CMDAT_AUTO_CMD12;
		auto_cmd12 = 1;
	}

	if(cmd->cmdidx == IMASK_END_CMD_RES)
		cmdat = DMAC_MODE_SEL;

	if(cmd->cmdidx == MMC_CMD_GO_IDLE_STATE)
		cmdat =	CMDAT_INIT;

	if(cmd->cmdidx == MMC_CMD_STOP_TRANSMISSION) {
		if(auto_cmd12) {
			auto_cmd12 = 0;
			if(wait_set_status(mmc_base, STAT_AUTO_CMD12_DONE)) {
				printf("%s: timedout waiting for CMD12 done\n", __func__);
				return -EBUSY;
			} else {
				return 0;
			}
		}else{
			cmdat =	0x41;
		}
	}
	writel(cmdat, &mmc_base->cmdat);
	writel(cmd->cmdidx, &mmc_base->cmd);
	writel(cmd->cmdarg, &mmc_base->arg);
	writel(CTRL_START_OP, &mmc_base->ctrl);

	if(wait_set_status(mmc_base, STAT_END_CMD_RES)) {
		printf("%s: timedout waiting for send command %d[ox%x]\n",
				__func__, cmd->cmdidx,cmd->cmdidx);
		return -EBUSY;
	}

	writel(IFLG_END_CMD_RES, &mmc_base->ireg);

	if (cmd->resp_type & MMC_RSP_PRESENT) {
		if (cmd->resp_type & MMC_RSP_136) {
			/* response type 2
			 * Linux says:
			 * Did I mention this is Sick.  We always need to
			 * discard the upper 8 bits of the first 16-bit word.
			 */
			res = read_mmc_res(mmc, 0);
			for(i = 0; i < 4; i++) {
				cmd->response[i] = res << 24;
				res = read_mmc_res(mmc, 0);
				cmd->response[i] |= res << 8;
				res = read_mmc_res(mmc, 0);
				cmd->response[i] |= res >> 8;
			}
		} else {
			/* response types 1, 1b, 3, 4, 5, 6 */
			cmd->response[0] = read_mmc_res(mmc, 1) << 8 |
					((read_mmc_res(mmc, 0)) && 0xff);
		}
	}

#if 0
	debug("MMC: send cmd %d[ox%x] arg ox%x cmdat ox%x stat ox%x\n",
			cmd->cmdidx, cmd->cmdidx, cmd->cmdarg, cmdat, mmc_base->stat);
	debug("response 0x%x 0x%x 0x%x 0x%x\n", cmd->response[0], cmd->response[1],
			cmd->response[2], cmd->response[3]);
#endif

	if(data) {
		if(data->flags & MMC_DATA_READ) {
			ret = mmc_read_data(mmc, (u32 *)data->dest,
					data->blocksize * data->blocks);
		} else if(data->flags & MMC_DATA_WRITE) {
			ret = mmc_write_data(mmc, (u32 *)data->dest,
					data->blocksize * data->blocks);
		}

		if(ret) {
			printf("sd card read and write error");
		} else
			ret = data_trans_done(mmc, data);
	}
 
	return ret;
}

static void mmc_set_ios(struct mmc *mmc)
{
	struct m200_mmc_host *host = (struct m200_mmc_host *)mmc->priv;
	struct hsmmc *mmc_base = host->regs;
	u32 real_rate = 0;
	u32 msc_lpm = 0;
	u8 clk_div = 0;

	if (mmc->clock > 1000000) {
		host->clk->ops->set_rate(host->clk, mmc->clock);
	}
	else {
		host->clk->ops->set_rate(host->clk, 24000000);
		mmc->clock = 24000000;
	}

	real_rate = host->clk->ops->get_rate(host->clk);

	while ((real_rate > mmc->clock) && (clk_div < 7)) {
		real_rate >>= 1;
		clk_div++;
	}

	if (mmc->bus_width == 4 || mmc->bus_width == 8)
		writel(clk_div, &mmc_base->clkrt);
	else
		writel(3, &mmc_base->clkrt);

	msc_lpm = readl(&mmc_base->lpm);
	if (real_rate > 25000000)
		msc_lpm |= (0x2 << LPM_DRV_SEL_SHF) | LPM_SMP_SEL;
	writel(msc_lpm, &mmc_base->lpm);
}

static int mmc_init_setup(struct mmc *mmc)
{
	struct m200_mmc_host *host = (struct m200_mmc_host *)mmc->priv;
	struct hsmmc *mmc_base = host->regs;

#if 0
	msc_gpio_func_init();
#endif
	writel(CTRL_RESET, &mmc_base->ctrl);
	if(wait_clear_status(mmc_base, STAT_IS_RESETTING)) {
		printf("%s: timedout waiting for reset mmc Control\n", __func__);
		return -EBUSY;
	}

	writel(0xffffff, &mmc_base->imask);
	writel(0xffffff, &mmc_base->ireg);

	writel(0xffffff, &mmc_base->rdto);
	writel(0xffff, &mmc_base->resto);
	writel(1, &mmc_base->lpm);

	return 0;
}

int m200_mmc_init(int dev_index, uint host_caps_mask, uint f_max)
{
	struct mmc *mmc;
	struct m200_mmc_host *host;

	mmc = &hsmmc_dev[dev_index];
	host = &mmc_host[dev_index];

	switch (dev_index) {
	case 0:
		host->regs = (struct hsmmc *)MSC0_REG_BASE;
		gpio_as_msc0_pa_4bit();
		break;
	case 1:
		host->regs = (struct hsmmc *)MSC1_REG_BASE;
		gpio_as_msc1_pe_4bit();
		break;
	case 2:
		host->regs = (struct hsmmc *)MSC2_REG_BASE;
		break;
	default:
		printf("No SD/MMC Controller %d!\n", dev_index);
		return -EINVAL;
	}

	sprintf(mmc->name, "msc%d", dev_index);

	host->clk = clk_get(mmc->name);

	if(!host->clk) {
		printf("%s requst clk error\n", mmc->name);
		return -ENOLCK;
	}

	clk_enable(host->clk);

	mmc->priv = host;

	mmc->send_cmd = mmc_send_cmd;
	mmc->set_ios = mmc_set_ios;
	mmc->init = mmc_init_setup;
	mmc->getcd = NULL;

	mmc->voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;
	mmc->version = SD_VERSION_2;
	mmc->host_caps = (MMC_MODE_4BIT | MMC_MODE_HS_52MHz | MMC_MODE_HS |
				MMC_MODE_HC) & ~host_caps_mask;

	mmc->f_min = 24000000;

	if (f_max != 0)
		mmc->f_max = f_max;
	else {
		if (mmc->host_caps & MMC_MODE_HS) {
			if (mmc->host_caps & MMC_MODE_HS_52MHz)
				mmc->f_max = 52000000;
			else
				mmc->f_max = 24000000;
		} else
			mmc->f_max = 24000000;
	}

	/* A maximum number of blocks to read and write */
	mmc->b_max = 0;

	mmc_register(mmc);

	return 0;
}

