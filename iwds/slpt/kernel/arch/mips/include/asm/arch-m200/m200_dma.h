/*
 *
 *  Copyright (C) 2013 Fighter Sun <wanmyqawdr@126.com>
 *  GPDMA controller driver for SoC-M200
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __M200_DMA_H__
#define __M200_DMA_H__

#include <common.h>
#include <asm/bitops.h>

#define NR_DMA_CHANNELS 32

typedef struct {
	volatile u32 dsa;
	volatile u32 dta;
	volatile u32 dtc;
	volatile u32 drt;
	volatile u32 dcs;
	volatile u32 dcm;
	volatile u32 dda;
	volatile u32 dsd;
} dma_chan_regs_file_t;

typedef struct {
	volatile u32 dmac;
	volatile u32 dirqp;
	volatile u32 ddr;
	volatile u32 ddrs;
	volatile const u32 __unused0;
	volatile const u32 __unused1;
	volatile const u32 __unused2;
	volatile u32 dmacp;
	volatile u32 dsirqp;
	volatile u32 dsirqm;
	volatile u32 dcirqp;
	volatile u32 dcirqm;
	volatile u32 dmcs;
	volatile u32 dmnmb;
	volatile u32 dmsmb;
	volatile u32 dmint;
} dma_ctrl_regs_file_t;

typedef struct {
	dma_chan_regs_file_t chan[NR_DMA_CHANNELS];

	uint32_t pad[(0x13421000 - 0x13420400) / sizeof(uint32_t)];

	dma_ctrl_regs_file_t ctrl;
} dmac_regs_file_t;

typedef struct {
	int id;
	dma_ctrl_regs_file_t *ctrl_regs_file;
	dma_chan_regs_file_t *chan_regs_file;
} m200_dma_chan_t;

typedef enum {
	JZDMA_REQ_RESERVED0 = 0x03,
	JZDMA_REQ_I2S1,
	JZDMA_REQ_I2S0,
	JZDMA_REQ_AUTO_TXRX = 0x08,
	JZDMA_REQ_SADC_RX,
	JZDMA_REQ_RESERVED1 = 0x0b,
	JZDMA_REQ_UART4,
	JZDMA_REQ_UART3,
	JZDMA_REQ_UART2,
	JZDMA_REQ_UART1,
	JZDMA_REQ_UART0,
	JZDMA_REQ_SSI0,
	JZDMA_REQ_SSI1,
	JZDMA_REQ_MSC0,
	JZDMA_REQ_MSC1,
	JZDMA_REQ_MSC2,
	JZDMA_REQ_PCM0,
	JZDMA_REQ_PCM1,
	JZDMA_REQ_I2C0,
	JZDMA_REQ_I2C1,
	JZDMA_REQ_I2C2,
	JZDMA_REQ_I2C3,
	JZDMA_REQ_I2C4,
	JZDMA_REQ_DES,
} m200_dma_req_type_t;

typedef enum {
	DMA_SLAVE_BUSWIDTH_UNDEFINED = 0,
	DMA_SLAVE_BUSWIDTH_1_BYTE = 1,
	DMA_SLAVE_BUSWIDTH_2_BYTES = 2,
	DMA_SLAVE_BUSWIDTH_4_BYTES = 4,
	DMA_SLAVE_BUSWIDTH_8_BYTES = 8,
} dma_slave_buswidth_t;


typedef enum {
	DMA_BIDIRECTIONAL = 0,
	DMA_TO_DEVICE = 1,
	DMA_FROM_DEVICE = 2,
	DMA_NONE = 3,
} dma_data_direction_t;

typedef struct {
	dma_data_direction_t direction;
	dma_addr_t src_addr;
	dma_addr_t dst_addr;
	dma_slave_buswidth_t src_addr_width;
	dma_slave_buswidth_t dst_addr_width;
	u32 src_maxburst;
	u32 dst_maxburst;
} dma_slave_config_t;


#define DMINT_S_IP              BIT(17)
#define DMINT_N_IP              BIT(16)

#define DMAC_HLT                BIT(3)
#define DMAC_AR                 BIT(2)

#define DCS_NDES                BIT(31)
#define DCS_AR                  BIT(4)
#define DCS_TT                  BIT(3)
#define DCS_HLT                 BIT(2)
#define DCS_CTE                 BIT(0)

#define DCM_SAI                 BIT(23)
#define DCM_DAI                 BIT(22)
#define DCM_SP_MSK              (0x3 << 14)
#define DCM_SP_32               DCM_SP_MSK
#define DCM_SP_16               BIT(15)
#define DCM_SP_8                BIT(14)
#define DCM_DP_MSK              (0x3 << 12)
#define DCM_DP_32               DCM_DP_MSK
#define DCM_DP_16               BIT(13)
#define DCM_DP_8                BIT(12)
#define DCM_TSZ_MSK             (0x7 << 8)
#define DCM_TSZ_SHF             8
#define DCM_STDE                BIT(2)
#define DCM_TIE                 BIT(1)
#define DCM_LINK                BIT(0)

#define DCM_CH1_SRC_TCSM        (0x0 << 26)
#define DCM_CH1_SRC_NEMC        (0x1 << 26)
#define DCM_CH1_SRC_DDR         (0x2 << 26)

#define DCM_CH1_DST_TCSM        (0x0 << 24)
#define DCM_CH1_DST_NEMC        (0x1 << 24)
#define DCM_CH1_DST_DDR         (0x2 << 24)

#define DCM_CH1_DDR_TO_NAND     (DCM_CH1_SRC_DDR  | DCM_CH1_DST_NEMC)
#define DCM_CH1_NAND_TO_DDR     (DCM_CH1_SRC_NEMC | DCM_CH1_DST_DDR)

#define DCM_CH1_TCSM_TO_NAND    (DCM_CH1_SRC_TCSM | DCM_CH1_DST_NEMC)
#define DCM_CH1_NAND_TO_TCSM    (DCM_CH1_SRC_NEMC | DCM_CH1_DST_TCSM)

#define DCM_CH1_TCSM_TO_DDR     (DCM_CH1_SRC_TCSM | DCM_CH1_DST_DDR)
#define DCM_CH1_DDR_TO_TCSM     (DCM_CH1_SRC_DDR  | DCM_CH1_DST_TCSM)

typedef int (*m200_dma_filter_t)(m200_dma_chan_t *dma_chan);

extern m200_dma_chan_t *m200_dma_alloc_chan(m200_dma_filter_t filter);
extern void m200_dma_release_chan(m200_dma_chan_t *chan);
extern void m200_dma_chan_dump(m200_dma_chan_t *chan);

#endif
