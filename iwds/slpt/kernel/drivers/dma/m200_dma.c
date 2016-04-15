/*
 *  Copyright (C) 2013 Fighter Sun <wanmyqawdr@126.com>
 *  GPDMA controller driver for SoC-4780
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

#include <common.h>
#include <linux/err.h>
#include <linux/compat.h>

#include <asm/bitops.h>
#include <asm/addrspace.h>

#include <asm/arch/cpm.h>

#if defined(CONFIG_JZ4780)
#include <asm/arch/jz4780_dma.h>
#elif defined(CONFIG_JZ4775)
#include <asm/arch/m200_dma.h>
#elif defined(CONFIG_M200)
#include <asm/arch/m200_dma.h>
#endif
#define DRVNAME "m200-dma"

#define GPDMA_REGS_FILE_BASE 0x13420000

/* instance a singleton dmac */
static struct {
	dmac_regs_file_t *regs_file;
	struct clk *dma_clk;

	m200_dma_chan_t chans[NR_DMA_CHANNELS];
	unsigned long long chan_use_map;

	int init_done;
} instance = {
	.regs_file = (dmac_regs_file_t *) CKSEG1ADDR(GPDMA_REGS_FILE_BASE),
}, *dmac = &instance; /* end instance a singleton dmac */


static int m200_dma_init_done(void)
{
	return dmac->init_done;
}

m200_dma_chan_t *m200_dma_alloc_chan(m200_dma_filter_t filter)
{
	if (!m200_dma_init_done()) {
		printk(DRVNAME": WARNING! GPDMA controller is not available.\n");
		return NULL;
	}

	int id;
	for (id = 0; id < NR_DMA_CHANNELS; id++) {
		if (!test_bit(id, &dmac->chan_use_map) &&
			filter(&dmac->chans[id])) {
			set_bit(id, &dmac->chan_use_map);

			printk(DRVNAME": channel(PHY ID: %d) has"
					" been requested.\n", id);
			return &dmac->chans[id];
		}
	}

	return NULL;
}

void m200_dma_release_chan(m200_dma_chan_t *chan)
{
	if (!m200_dma_init_done()) {
		printk(DRVNAME": WARNING! GPDMA controller is not available.\n");
		return;
	}

	if (!test_bit(chan->id, &dmac->chan_use_map)) {
		printk(DRVNAME": WARNING! can not release a unallocted channel.\n");
		return;
	}

	clear_bit(chan->id, &dmac->chan_use_map);
}

void m200_dma_chan_dump(m200_dma_chan_t *chan)
{
	dma_chan_regs_file_t *chan_regs = chan->chan_regs_file;
	dma_ctrl_regs_file_t *ctrl_regs = chan->ctrl_regs_file;

	printk("\n");
	printk("CH_DSA = 0x%08x\n", chan_regs->dsa);
	printk("CH_DTA = 0x%08x\n", chan_regs->dta);
	printk("CH_DTC = 0x%08x\n", chan_regs->dtc);
	printk("CH_DRT = 0x%08x\n", chan_regs->drt);
	printk("CH_DCS = 0x%08x\n", chan_regs->dcs);
	printk("CH_DCM = 0x%08x\n", chan_regs->dcm);
	printk("CH_DDA = 0x%08x\n", chan_regs->dda);
	printk("CH_DSD = 0x%08x\n", chan_regs->dsd);
	printk("DMAC   = 0x%08x\n", ctrl_regs->dmac);
	printk("DIRQP  = 0x%08x\n", ctrl_regs->dirqp);
	printk("DDR    = 0x%08x\n", ctrl_regs->ddr);
	printk("DDRS   = 0x%08x\n", ctrl_regs->ddrs);
	printk("DMACP  = 0x%08x\n", ctrl_regs->dmacp);
	printk("DSIRQP = 0x%08x\n", ctrl_regs->dsirqp);
	printk("DSIRQM = 0x%08x\n", ctrl_regs->dsirqm);
	printk("DCIRQP = 0x%08x\n", ctrl_regs->dcirqp);
	printk("DCIRQM = 0x%08x\n", ctrl_regs->dcirqm);
}

int m200_dma_init(void)
{
	int id;
	for (id = 0; id < NR_DMA_CHANNELS; id++) {
		dmac->chans[id].id = id;
		dmac->chans[id].ctrl_regs_file = &dmac->regs_file->ctrl;
		dmac->chans[id].chan_regs_file = &dmac->regs_file->chan[id];
	}

	dmac->dma_clk = clk_get("pdma");
	if (IS_ERR(dmac->dma_clk)) {
		printk(DRVNAME": Failed to get clk pdma.\n");
		return PTR_ERR(dmac->dma_clk);
	}
	clk_enable(dmac->dma_clk);

	/*
	 * set all channels controlled by main core
	 */
	dmac->regs_file->ctrl.dmacp = 0;

	/*
	 * global DMA enable
	 */
	dmac->regs_file->ctrl.dmac |= BIT(0);

	dmac->init_done = 1;

	printk(DRVNAME": SoC-4780 GPDMA controller initialized.\n");
	return 0;
}
