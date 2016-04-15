/*
 * Copyright (C) 2013 Ingenic Semiconductor Co., Ltd.
 * Authors: Kage Shen <kkshen@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the License, or (at your
 * option) any later version.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>
#include <common.h>
#include <malloc.h>

#include <linux/err.h>

#include <asm/io.h>
#include <asm/arch/mmc.h>
#include <asm/arch/cpm.h>
#include <asm/arch/udc.h>

#include <usbdevice.h>

#include <usb/jz4780_udc.h>

#include <asm/arch/board_special.h>

#include "ep0.h"
#include "usb_cdc_acm.h"
#include "usbdescriptors.h"

#if 0
volatile u32 gflag;
#define b1_flag (1)
#define b2_flag (1<<1)
#define b3_flag (1<<2)
#define b4_flag (1<<3)
#define b5_flag (1<<4)
#define b6_flag (1<<5)
#define b7_flag (1<<6)
#define if_6flag() if (gflag & b6_flag)
#define set_6flag() do {gflag |= b6_flag;} while(0)
#define clean_6flag() do {gflag &= (~b6_flag);} while(0)
#endif

//#undef CONFIG_USB_DMA
//#define CONFIG_USB_DMA

#undef usbinfo
#undef usbdbg
#undef usberr

#define usbinfo(fmt, args...) serial_printf("udc: "fmt, ##args)
#define usberr(fmt, args...) serial_printf("\033[1;31m udc: " fmt "\033[0m", ##args);
#define usb_print(fmt, args...) serial_printf(fmt, ##args)

#if 0
#define usbdbg(fmt,args...) serial_printf(fmt, ##args)
#define usbmsg(fmt,args...) serial_printf(fmt, ##args)
#else
#define usbdbg(fmt,args...) do{}while(0)
#define usbmsg(fmt,args...) do{}while(0)
#endif

#define EPST_ENABLE		1
#define EPST_DISABLE	0
#define EPST_NAK	0
#define EPST_ACK	1

#define EP0_PKTMASK (0x1f)
#define OEP0_PHY_PKTSIZE (3 * 8)
#define IEP0_PHY_PKTSIZE (64)
#define OEP0_DMA_PKTSIZE (3 * 8)
#define IEP0_DMA_PKTSIZE (64 * 2)

#define TRANSFER_TIME_OUT 10

#define EPx_PKTMASK (0x3ffff)
/* EPx max packet size = sizeof(urb.buffer)  */
#define EPx_DMA_PKTSIZE (512 * 2 * 3)

#define set_oep_status(ep, en, ack) do {                \
	u32 status = readl(DOEP_CTL(ep));                   \
	status |= (en ? DEP_ENA_BIT : DEP_DISENA_BIT);      \
	status |= (ack ? DEP_CLEAR_NAK : DEP_SET_NAK);      \
	writel(status, DOEP_CTL(ep));                       \
} while(0)

#define set_iep_status(ep, en, ack) do {                \
	u32 status = readl(DIEP_CTL(ep));                   \
	status |= (en ? DEP_ENA_BIT : DEP_DISENA_BIT);      \
	status |= (ack ? DEP_CLEAR_NAK : DEP_SET_NAK);      \
	writel(status, DIEP_CTL(ep));                       \
} while(0)

volatile int enable_ep0_out_flag = 0;
#define set_quick_enable_ep0() do {enable_ep0_out_flag = 1;} while(0)
#define clean_quick_enable_ep0() do {enable_ep0_out_flag = 0;} while(0)
#define is_quick_enable_ep0() (enable_ep0_out_flag ? 1 : 0)

struct ep_attribute {
	u32 transfer_size;
	u32 receive_size;
	u32 ep_pktsize;
	/* A IN interrupt handle transmission MAX packet size */
	u32 mxirq_inep_pktsize;
	/* A OUT interrupt handle transmission MAX packet size */
	u32 mxirq_outep_pktsize;
	u32 backup_inep_pktsize;
	u32 backup_outep_pktsize;
	void *pbuffer;
};

struct jz4780_udc_dev {
	char *name;
	struct clk *clk;
	struct urb *ep0_urb;
	struct list_head pipe_list;
	struct udc_pipe_dev *base_pipe_dev;
	struct udc_pipe_dev *backup_pipe_dev;
	struct ep_attribute ep_attr[MAX_ENDPOINTS];
	u32 data_out_epnum;
	u32 data_out_block;
	u32 data_in_epnum;
	u32 data_in_timeout;
	u32 data_in_echo;
};

static struct jz4780_udc_dev *udc_dev;

#if defined(CONFIG_JZ4780_USB)
extern struct jz4780_cpm_regs *cpm_reg;
#elif defined(CONFIG_JZ4775_USB)
extern struct jz4775_cpm_regs *cpm_reg;
#endif

struct dwc_otg_core_global_regs *otg_reg =
		(struct dwc_otg_core_global_regs *)OTG_BASE;

static void otg_core_reset(void)
{
	u32 time_out;
	/* reset otg */
	writel(readl(&otg_reg->grstctl) | GRSTCTL_CORE_RST, &otg_reg->grstctl);

	time_out = get_timer(0);
	while(get_timer(time_out) < 500 &&
			(readl(&otg_reg->grstctl) & GRSTCTL_CORE_RST));
	if(get_timer(time_out) >= 500) {
		usbinfo("otg reset timeout\n");
	}

	/*
	time_out = get_timer(0);
	while(get_timer(time_out) < 500 &&
			(readl(&otg_reg->gintsts) & GINTSTS_USB_EARLYSUSPEND));
	if(get_timer(time_out) >= 500) {
		usbinfo("otg reset interrupt timeout\n");
	} else {
		writel(GINTSTS_USB_EARLYSUSPEND, &otg_reg->gintsts);
	}
	 * */
}

static void otg_flush_rx_fifo(void)
{
	u32 time_out;

	writel(GRSTCTL_RXFIFO_FLUSH, &otg_reg->grstctl);

	time_out = get_timer(0);
	while(get_timer(time_out) < 500 &&
			(readl(&otg_reg->grstctl) & GRSTCTL_RXFIFO_FLUSH));

	if(get_timer(time_out) >= 500) {
		usbinfo("otg flush rx fifo timeout\n");
	}
}

static void otg_flush_tx_fifo(int epnum)
{
	u32 time_out;
	/* otg flush tx fifo */
	writel((epnum << 6) | GRSTCTL_TXFIFO_FLUSH, &otg_reg->grstctl);

	time_out = get_timer(0);
	while(get_timer(time_out) < 500 &&
			(readl(&otg_reg->grstctl) & GRSTCTL_TXFIFO_FLUSH));

	if(get_timer(time_out) >= 500) {
		usbinfo("otg flush tx fifo timeout\n");
	}
}

static void otg_interrupt_mask(void)
{
	writel(0, DOEP_MASK);
	writel(0, DIEP_MASK);
	writel(0xffffffff, OTG_DAINT);
	writel(0, DAINT_MASK);
	writel(0xffffffff, &otg_reg->gintsts);

	writel(readl(&otg_reg->gintmsk) | (1 << 4), &otg_reg->gintmsk);
}

static void otg_ep0_init(void)
{
	u32 usb_reg_t;

	usb_reg_t = DEP_DISENA_BIT | DEP_SET_NAK;

	writel(readl(DIEP_CTL(0)) | usb_reg_t, DIEP_CTL(0));
	writel(readl(DOEP_CTL(0)) | usb_reg_t, DOEP_CTL(0));

	writel(0, OTG_DCFG);
	writel(0, OTG_DCTL);
}

static void otg_fifo_config(void)
{
	usbdbg("otg_fifo_config:\n%x\n%x\n%x\n%x\n",
			readl(&otg_reg->ghwcfg1),
			readl(&otg_reg->ghwcfg2),
			readl(&otg_reg->ghwcfg3),
			readl(&otg_reg->ghwcfg4));

	/* set otg RX/TX fifo size */
	{
		u32 dev_rx_fifo_size;
		u32 host_rx_fifo_size;
		u32 dev_in_ep_tx_fifo_size0;
		u32 host_non_periodic_tx_fifo_size;
		u32 ep0_tx_fifo_size;
		u32 rx_fifo_size;
		u32 num_in_eps;
		u32 txfifo_start_addr;
		u32 i;

		u32 const x = 1;
		/* NOTE: each fifo max depth is 3576 (NOTE: see register reset value) */
		dev_rx_fifo_size = (4 * 1 + 6) + (x + 1) * (1024 / 4 + 1) +
			(2 * ((readl(&otg_reg->ghwcfg2) >> 10) & 0xf)) + 1;
		host_rx_fifo_size = (1024 / 4) + 1 + 1 + 1 * MAX_ENDPOINTS;
		rx_fifo_size = max(dev_rx_fifo_size, host_rx_fifo_size);
		/* Rx starting address fixed to 0,
		 * its depth is now configured to rx_fifo_size */
		writel(rx_fifo_size, &otg_reg->grxfsiz);

		/* GNPTXFSIZ if used by EP0, its start address is rx_fifo_size */
		dev_in_ep_tx_fifo_size0 = (x + 1) * (64 / 4);
		host_non_periodic_tx_fifo_size = (x + 1) * (512 / 4);
		ep0_tx_fifo_size = max(dev_in_ep_tx_fifo_size0,
				host_non_periodic_tx_fifo_size);

		writel((ep0_tx_fifo_size << 16) | rx_fifo_size, &otg_reg->gnptxfsiz);
		usbdbg("grxfsiz %x gnptxfsiz %x\n", otg_reg->grxfsiz, otg_reg->gnptxfsiz);
		/* configure EP1~n FIFO start address and depth */
		txfifo_start_addr = ep0_tx_fifo_size + rx_fifo_size;

		num_in_eps = (readl(&otg_reg->ghwcfg4) >> 26) & 0xf;
		for(i = 0; i < num_in_eps; i++) {
			u32 tmp_fifo_size = ((x + 1) * (512 / 4));
			writel(tmp_fifo_size, &otg_reg->dtxfsiz[i]);
			txfifo_start_addr += tmp_fifo_size;
			usbdbg("dtxfsiz[%d] %x\n", i, otg_reg->dtxfsiz[i]);
		}
		/*
		 * configure FIFO start address and depth
		 * for Endpoint Information Controller
		 */
		txfifo_start_addr = (txfifo_start_addr << 16) |
				(readl(&otg_reg->ghwcfg3) >> 16);
		writel(txfifo_start_addr, &otg_reg->gdfifocfg);

		usbdbg("gdfifocfg %x\n", otg_reg->gdfifocfg);
	}

	otg_flush_rx_fifo();
	otg_flush_tx_fifo(0x10);
}

static void otg_phy_init(void)
{
	u32 usb_reg_t;
	u32 time_out;

	/* dwc otg core init */
	usb_reg_t = readl(&otg_reg->gusbcfg);
	/*
	 * 4:0 UTMI+ Interface 22:0 Data line pulsing using utmi_txvalid (default)
	 * 17:0 ULPI interface 19:0 PHY powers down internal clock during suspend
	 * 9:1 HNP capability is enabled 8:1 SRP capability is enabled.
	 * */
	usb_reg_t &= ~((1 << 4) | (1 << 6) | (1 << 8) | (1 << 9) | (1 << 5));
	writel(usb_reg_t, &otg_reg->gusbcfg);

	if((((readl(&otg_reg->ghwcfg2) >> 6) & 3) == 1) &&
			(((readl(&otg_reg->ghwcfg4) >> 14) & 3) == 2)){

		writel(readl(&otg_reg->gusbcfg) | GUSBCFG_PHY_16BIT |
				GUSBCFG_TRDTIME_9, &otg_reg->gusbcfg);

		/* select utmi data bus width of port0 to 16bit/30M */
		writel(readl(&cpm_reg->usbpcr1) | USBPCR1_WORD_IF0 |
				USBPCR1_WORD_IF1, &cpm_reg->usbpcr1);
	} else {
		usberr("otg phy type width error !\n");
	}

	/* wait otg entry idle mode */
	time_out = get_timer(0);

	while(get_timer(time_out) < 500 &&
			(readl(&otg_reg->grstctl) & GRSTCTL_AHB_IDLE) == 0);

	if(get_timer(time_out) >= 500) {
		usberr("otg wait idle timeout\n");
	}

	otg_core_reset();

	writel(1 << 7, &otg_reg->gahbcfg);
	writel(0, &otg_reg->gintmsk);
}

static void otg_cpm_init(void)
{
	u32 usb_reg_t;
	u32 usb_clk_div;

	/* select dwc otg (synopsys) */
	writel(readl(&cpm_reg->usbpcr1) | USBPCR1_USB_SEL,
			&cpm_reg->usbpcr1);

	usb_clk_div = misc_param.extal_clock / 24;

	usb_reg_t = readl(&cpm_reg->usbpcr1);
	usb_reg_t &= ~USBPCR1_REFCLK_MASK;
	usb_reg_t |= (usb_clk_div << USBPCR1_REFCLK_DEV);
	/* set usb clk divide */
	writel(usb_reg_t, &cpm_reg->usbpcr1);

	usb_reg_t = readl(&cpm_reg->usbpcr) | USBPCR_VBUSVLDEXT;
	/* work as USB device mode */
	writel(usb_reg_t & ~USBPCR_OTG_MODE, &cpm_reg->usbpcr);
//	writel(0xc5893fdf, &cpm_reg->usbpcr);

	writel(0, &cpm_reg->usbvbfil);
	/* set that bits control USB reset detect time. */
	writel(0x96 | (1 << 25), &cpm_reg->usbrdt);

	/* otg power on reset */
	writel(readl(&cpm_reg->usbpcr) | USBPCR_POR, &cpm_reg->usbpcr);
	mdelay(1);
	writel(readl(&cpm_reg->usbpcr) & ~USBPCR_POR, &cpm_reg->usbpcr);
	mdelay(10);

	/* enable tx pre-emphasis, OTGTUNE adjust */
	writel(readl(&cpm_reg->usbpcr) | (1 << 6) | (7 << 14), &cpm_reg->usbpcr);
	/* port0(otg) hasn't forced to entered SUSPEND mode */
	writel(readl(&cpm_reg->opcr) | (1 << 7), &cpm_reg->opcr);
}

static inline u32 otg_get_status(void)
{
	return readl(&otg_reg->gintsts);
}

static inline void otg_set_address(u32 value)
{
	writel((value & 0xff) << DEV_ADDR_BIT, OTG_DCFG);
}

#ifndef CONFIG_USB_DMA
static u32 otg_pio_read_fifo(u32 ep_num, void *buf, u32 count)
{
	u32 i;
	u32 num = (count + 3) / 4;

	if (!count)
		return 0;

	if(((u32)buf & 3) == 0) {
		for (i = 0; i < num; i++) {
			*((u32 *)buf + i) = readl(EP_FIFO(ep_num));
		}
	} else {
		volatile u32 dat;
		for (i = 0; i < num; i++) {
			dat = readl(EP_FIFO(ep_num));
			*((u8 *)buf + i * 4 + 0) = dat & 0xff;
			*((u8 *)buf + i * 4 + 1) = (dat >> 8) & 0xff;
			*((u8 *)buf + i * 4 + 2) = (dat >> 16) & 0xff;
			*((u8 *)buf + i * 4 + 3) = (dat >> 24) & 0xff;
		}
	}

	return count;
}

static u32 otg_pio_write_fifo(u32 ep_num, void *buf, u32 count)
{
	u32 i;
	u32 num = (count + 3) / 4;

	if(((u32)buf & 3) == 0) {
		for (i = 0; i < num; i++) {
			writel(*((u32 *)buf + i), EP_FIFO(ep_num));
		}
	} else {
		volatile u32 dat;
		for (i = 0; i < num; i++) {
			dat =  (u32)*((u8 *)buf + i * 4 + 0) & 0xff;
			dat |=  ((u32)*((u8 *)buf + i * 4 + 1) << 8) & (0xff << 8);
			dat |=  ((u32)*((u8 *)buf + i * 4 + 2) << 16) & (0xff << 16);
			dat |=  ((u32)*((u8 *)buf + i * 4 + 3) << 24) & (0xff << 24);
			writel(dat, EP_FIFO(ep_num));
		}
	}
	return count;
}
#endif

static inline void udc_update_out_clannel(u32 ep_num, void *addr, u32 size)
{
	struct ep_attribute *ep_attr;
	u32 pktcnt;

	ep_attr = &udc_dev->ep_attr[ep_num];
	ep_attr->pbuffer = addr;
	ep_attr->backup_outep_pktsize = size;
	pktcnt = (size + ep_attr->ep_pktsize - 1) / ep_attr->ep_pktsize;
	writel((dma_addr_t)CPHYSADDR(ep_attr->pbuffer), DOEP_DIEPDMA(ep_num));
	writel(size | (pktcnt << 19), DOEP_SIZE(ep_num));
}

static void udc_reset_status(void)
{
	struct usb_device_instance *device;

	device = udc_dev->ep0_urb->device;
	usbd_device_event_irq(device, DEVICE_BUS_INACTIVE, 0);
	usbd_device_event_irq(device, DEVICE_RESET, 0);
}

int jz4780_udc_init(void)
{
	u32 usb_reg_t;

	usbinfo("Jz4780 usbd start\n");

	udc_dev = malloc(sizeof(struct jz4780_udc_dev));
	if (!udc_dev) {
		usbinfo("%s malloc memary error\n", udc_dev->name);
		return -EBUSY;
	}

	memset(udc_dev, 0, sizeof(struct jz4780_udc_dev));

	/* otg1 is synopsys otg */
	udc_dev->name = "otg1";

	udc_dev->clk = clk_get(udc_dev->name);

	if(!udc_dev->clk) {
		usbinfo("%s requst clk error\n", udc_dev->name);
		return -ENOLCK;
	}

	clk_enable(udc_dev->clk);

	otg_cpm_init();

	otg_phy_init();

	otg_fifo_config();

	otg_ep0_init();

	otg_interrupt_mask();
#ifdef CONFIG_USB_DMA
	writel(readl(&otg_reg->gahbcfg) | 1 | (1 << 5),
			&otg_reg->gahbcfg);
	usbinfo("usb enable dma mode !\n");
#else
	usbinfo("usb enable pio mode !\n");
	writel(readl(&otg_reg->gahbcfg) | 1, &otg_reg->gahbcfg);
#endif

	writel(readl(DAINT_MASK) | (1 << 4) |
			(1 << 28) | (1 << 19) | (1 << 18)
			| (1 << 13) | (1 << 12), DAINT_MASK);

	/* check synopsys ID */
	usb_reg_t = readl(&otg_reg->gsnpsid);

	if((usb_reg_t & 0xffff0000) != 0x4f540000) {
		usberr("synopsys id read error\n");
		return -ENODEV;
	}

	INIT_LIST_HEAD(&udc_dev->pipe_list);

	return 0;
}

static void handle_early_suspend_intr(void)
{
	usbdbg("func: %s\n", __func__);

	otg_ep0_init();

	otg_flush_tx_fifo(0x10);	/* flush all tx endprint fifo */

	writel(GINTSTS_USB_EARLYSUSPEND, &otg_reg->gintsts);
	return;
}

static void handle_suspend_intr(void)
{
	struct usb_device_instance *device;

	usbdbg("func: %s\n", __func__);

	device = udc_dev->ep0_urb->device;
	if(device->device_state >= STATE_ADDRESSED) {
		udc_reset_status();
		otg_core_reset();
		otg_fifo_config();
	}

	writel(GINTSTS_USB_SUSPEND, &otg_reg->gintsts);
}

static void handle_reset_intr(void)
{
	struct usb_endpoint_instance *ep;
	struct usb_device_instance *device;
	struct ep_attribute *ep_attr;
	u32 ep_addr;
	u32 ep_num;
	u32 i;

	usbdbg("func: %s\n", __func__);
	device = udc_dev->ep0_urb->device;

	writel(readl(DAINT_MASK) | (1 << 0) | (1 << 16), DAINT_MASK);
	writel(readl(DOEP_MASK) | (1 << 0) | (1 << 3), DOEP_MASK);
	writel(readl(DIEP_MASK) | (1 << 0) | (1 << 3), DIEP_MASK);

	/* set nak with each endpoint except ep0 */
	for(i = 1; i < device->bus->max_endpoints; i++) {
		ep = device->bus->endpoint_array + i;
		ep_addr = ep->endpoint_address;
		ep_num = ep_addr & USB_ENDPOINT_NUMBER_MASK;

		if ((ep_addr & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT) {
			set_oep_status(ep_num, EPST_DISABLE, EPST_NAK);
			writel(0xff, DOEP_INT(ep_num));
		} else {
			set_iep_status(ep_num, EPST_DISABLE, EPST_NAK);
			writel(0xff, DIEP_INT(ep_num));
		}
	}

	/* flush all txfifos */
	otg_flush_tx_fifo(0x10);

	/* Reset Device Address */
	writel(readl(OTG_DCFG) & (~DEV_ADDR_MASK), OTG_DCFG);

#ifdef CONFIG_USB_DMA
	writel(readl(&otg_reg->gahbcfg) | 1 | (1 << 5),
			&otg_reg->gahbcfg);
#endif

	ep_attr = &udc_dev->ep_attr[0];

	/* setup EP0 to receive SETUP packets */
	writel(DOEPSIZE0_SUPCNT_3 | DOEPSIZE0_PKTCNT_BIT | 
			OEP0_PHY_PKTSIZE, DOEP_SIZE(0));

#ifdef CONFIG_USB_DMA
	writel((dma_addr_t)CPHYSADDR(ep_attr->pbuffer),
			DOEP_DIEPDMA(0));
#endif

	ep_attr->ep_pktsize = IEP0_PHY_PKTSIZE;
	ep_attr->backup_outep_pktsize = OEP0_PHY_PKTSIZE;

	ep_attr->mxirq_outep_pktsize = OEP0_PHY_PKTSIZE;
	ep_attr->mxirq_inep_pktsize = IEP0_DMA_PKTSIZE;

	writel(DEP_ENA_BIT | DEP_CLEAR_NAK, DOEP_CTL(0));
	writel(DEP_DISENA_BIT | DEP_SET_NAK, DIEP_CTL(0));

	writel(GINTSTS_USB_RESET, &otg_reg->gintsts);

	writel(0, DIEP_EMPMSK);
	return;
}

static inline void handle_start_frame_intr(void)
{
	writel(GINTSTS_START_FRAM, &otg_reg->gintsts);
}

static void handle_enum_done_intr(void)
{
	struct usb_endpoint_instance *ep;
	struct usb_device_instance *device;
	u32 usb_reg;
	u32 ep_spd;
	u32 ep_addr;
	u32 ep_num;
	u32 i;

	device = udc_dev->ep0_urb->device;
	usb_reg = readl(DIEP_CTL(0)) & (~3);
	ep_spd = readl(OTG_DSTS) & (3 << 1);

	switch(ep_spd) {
	case DSTS_ENUM_SPEED_HIGH:
		usbdbg("usb high speed\n");
		ep_spd = DEP_HS_PKTSIZE;
		break;
	case DSTS_ENUM_SPEED_FULL_30OR60:
	case DSTS_ENUM_SPEED_FULL_48:
		usbdbg("usb full speed\n");
		ep_spd = DEP_FS_PKTSIZE;
		break;
	case DSTS_ENUM_SPEED_LOW:
		usberr("usb low speed ??\n");
		usb_reg |= DEP_EP0_MPS_8;
		break;
	default:
		usbdbg("Fault speed enumration\n");
		break;
	}

	/* set packet size with each endpoint except ep0 */
	for(i = 1; i < device->bus->max_endpoints; i++) {
		ep = device->bus->endpoint_array + i;
		ep_addr = ep->endpoint_address;
		ep_num = ep_addr & USB_ENDPOINT_NUMBER_MASK;

		if ((ep_addr & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT) {
			writel(readl(DOEP_CTL(ep_num)) & (~DEP_PKTSIZE_MASK),
					DOEP_CTL(ep_num));
			writel(readl(DOEP_CTL(ep_num)) & (~DEP_TYPE_MASK),
					DOEP_CTL(ep_num));
			writel(readl(DOEP_CTL(ep_num)) | USB_ACTIVE_EP |
					(ep->rcv_attributes << 18) | ep_spd, DOEP_CTL(ep_num));
		} else {
			writel(readl(DIEP_CTL(ep_num)) & (~DEP_PKTSIZE_MASK),
					DIEP_CTL(ep_num));
			writel(readl(DIEP_CTL(ep_num)) & (~DEP_TYPE_MASK),
					DIEP_CTL(ep_num));
			writel(readl(DIEP_CTL(ep_num)) | USB_ACTIVE_EP |
					(ep->tx_attributes << 18) | ep_spd, DIEP_CTL(ep_num));
		}

		udc_dev->ep_attr[ep_num].ep_pktsize = ep_spd;
		udc_dev->ep_attr[ep_num].mxirq_outep_pktsize = ep_spd;
		udc_dev->ep_attr[ep_num].mxirq_inep_pktsize = ep_spd;
	}
#ifdef CONFIG_USB_DMA
	writel(usb_reg | DEP_ENA_BIT | DEP_SET_NAK, DIEP_CTL(0));
#else
	writel(usb_reg, DIEP_CTL(0));
#endif

	writel(readl(DOEP_CTL(0)) | DEP_ENA_BIT, DOEP_CTL(0));

	writel(readl(OTG_DCTL) | DCTL_CLR_GNPINNAK, OTG_DCTL);

	writel(GINTSTS_ENUM_DONE, &otg_reg->gintsts);
}

#ifdef CONFIG_USB_DMA
static void handle_dma_outep_intr(u32 ep_num, u32 status)
{
	struct ep_attribute *ep_attr;
	u32 count;

	ep_attr = &udc_dev->ep_attr[ep_num];
	count = ep_attr->backup_outep_pktsize - (readl(DOEP_SIZE(ep_num))
			& (ep_num ? EPx_PKTMASK : EP0_PKTMASK));

	invalidate_dcache_range((ulong)ep_attr->pbuffer,
			(ulong)ep_attr->pbuffer + count);

	switch(status) {
	case DEP_XFER_COMP:
		usbdbg("DMA Transfer count[%x] %x %d %x\n", readl(DOEP_DIEPDMA(ep_num)),
				readl(DOEP_SIZE(ep_num)), count, readl(DOEP_INT(ep_num)));
		ep_attr->receive_size = count;
		break;
	case DEP_STATUS_PHASE_RECV:
		usbdbg("STATUS_PHASE_RECV %d\n", count);
		break;
	case DEP_SETUP_PHASE_DONE:
		usbdbg("STATUS_PHASE_DONE %d\n", count);
		ep_attr->receive_size = count;
		{
			u32 *buf;
			u32 *data;

			if (count > 8) {
				u32 tmp_addr = (u32)readl(DOEP_DIEPDMA(0)) -
					(u32)CPHYSADDR(ep_attr->pbuffer);
				buf = (u32 *) ((u32)ep_attr->pbuffer - 8 + tmp_addr);
			} else {
				buf = (u32 *)ep_attr->pbuffer;
			}

			data = (u32 *) &udc_dev->ep0_urb->device_request;
			memcpy(data, buf, 8);
			usbdbg("usb receive setup packet[%x] %x %x\n",
					(u32)buf, data[0], data[1]);
			/* bmRequestType.Direction: Host-to-device
			 * wLength != 0
			 */
			if(!(data[0] & 0x80) && ((data[1] & 0xff0000) != 0)) {
				set_quick_enable_ep0();
			} else {
				clean_quick_enable_ep0();
			}
		}

		writel(DOEPSIZE0_SUPCNT_3 | DOEPSIZE0_PKTCNT_BIT |
			OEP0_PHY_PKTSIZE , DOEP_SIZE(0));

		writel((dma_addr_t)CPHYSADDR(udc_dev->ep_attr[0].pbuffer),
				DOEP_DIEPDMA(0));

		udc_dev->ep_attr[0].backup_outep_pktsize = OEP0_PHY_PKTSIZE;

		break;
	}
}

#else
static void handle_pio_outep_intr(u32 ep_num, u32 status, u32 rxstsp)
{
	struct ep_attribute *ep_attr;

	ep_attr = &udc_dev->ep_attr[ep_num];

	switch(status) {
	case GRXSTSP_PKSTS_SETUP_RECV:
	case GRXSTSP_PKSTS_GOUT_RECV: {
			void *p;
			u32 offset = 0;
			u32 count = (rxstsp >> GRXSTSP_BYTE_CNT_BIT) & GRXSTSP_BYTE_CNT_MASK;
			ep_attr->receive_size = count;
			if (ep_num == 0 && count != 8)
				offset = 8;
			otg_pio_read_fifo(ep_num,
					(void *)((u32)ep_attr->pbuffer + offset), count);
			if (ep_num == 0)
				break;
			p = pipe_push_to_dev(udc_dev->base_pipe_dev,
					ep_attr->receive_size);
			udc_update_out_clannel(ep_num, p,
					udc_dev->ep_attr[ep_num].ep_pktsize);
		}
		break;
	case DEP_SETUP_PHASE_DONE: {
			u32 *buf;
			u32 *data;

			buf = (u32 *)ep_attr->pbuffer;

			data = (u32 *) &udc_dev->ep0_urb->device_request;
			memcpy(data, buf, 8);
			usbdbg("usb receive setup packet[%x] %x %x\n",
					(u32)buf, data[0], data[1]);
		}
		break;
	default:
		break;
	}
}
#endif

static void handle_setup_packet(u32 ep_num)
{
	struct usb_device_instance *device;
	struct usb_device_request *request;
	struct ep_attribute *ep_attr;

	u32 i;

	ep_attr = &udc_dev->ep_attr[ep_num];
	device = udc_dev->ep0_urb->device;
	request = &udc_dev->ep0_urb->device_request;
	usbdbg("##SETPAG: rtype ox%x R ox%x Va ox%x In ox%x Le ox%x##\n",
			udc_dev->ep0_urb->device_request.bmRequestType,
			udc_dev->ep0_urb->device_request.bRequest,
			udc_dev->ep0_urb->device_request.wValue,
			udc_dev->ep0_urb->device_request.wIndex,
			udc_dev->ep0_urb->device_request.wLength );

	/* Try to process setup packet */
	udc_dev->ep0_urb->actual_length = 0;
	ep0_recv_setup(udc_dev->ep0_urb);
	ep_attr->transfer_size = udc_dev->ep0_urb->actual_length;
	ep_attr->receive_size = 0;
#ifndef CONFIG_USB_DMA
	if (ep_attr->receive_size > 127)
		usberr ("Warn: %d has exceeded the size of ep0 maximum length !\n",
				ep_attr->receive_size);
#endif

	/* Not a setup packet, stall next EP0 transaction */
	switch(request->bRequest) {
	case USB_REQ_SET_ADDRESS: {
		u32 value;

		usbd_device_event_irq(device, DEVICE_ADDRESS_ASSIGNED, 0);
		value = le16_to_cpu (request->wValue);
		otg_set_address(value);
		} break;
	case USB_REQ_SET_CONFIGURATION:
		for(i = 1; i < device->bus->max_endpoints; i++) {
			u32 ep_num, ep_addr;
			struct usb_endpoint_instance *ep;
			ep = device->bus->endpoint_array + i;
			ep_addr = ep->endpoint_address;
			ep_num = ep_addr & USB_ENDPOINT_NUMBER_MASK;

			if ((ep_addr & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT) {
				set_oep_status(ep_num, EPST_ENABLE, EPST_ACK);
			} else {
				set_iep_status(ep_num, EPST_ENABLE, EPST_NAK);
			}
		}

		usbd_device_event_irq(device, DEVICE_CONFIGURED, 0);

		if (udc_dev->base_pipe_dev) {
			void *p = pipe_push_to_dev(udc_dev->base_pipe_dev, 0);
			if (!p) {
				usberr("pipe_push_to_dev return NULL error !\n");
			} else {
				udc_update_out_clannel(udc_dev->data_out_epnum, p,
						udc_dev->ep_attr[udc_dev->data_out_epnum].ep_pktsize);
			}
		}

		break;
	case ACM_GET_LINE_ENCODING:
		/* 0x21 request DTE rate, stop/parity bits */
		break;
	case ACM_SET_LINE_ENCODING:
		/* 0x20 ACM request DTE rate, stop/parity bits */
		break;
	case ACM_SET_CONTROL_LINE_STATE:
		/* 0x22 Implies DTE ready */
		break;
	default:
		/* usberr ("can't parse setup packet, still waiting for setup\n"); */
		break;
	}
}

static void handle_data_phase(u32 ep_num)
{
	struct ep_attribute *ep_attr;
	u32 pktcnt;
	u32 remain_size;
#ifndef CONFIG_USB_DMA
	u32 time_out;
#endif

	ep_attr = &udc_dev->ep_attr[ep_num];
	remain_size = ep_attr->transfer_size - ep_attr->receive_size;

	if (remain_size) {
		if (remain_size > ep_attr->mxirq_inep_pktsize)
			ep_attr->backup_inep_pktsize = ep_attr->mxirq_inep_pktsize;
		else
			ep_attr->backup_inep_pktsize = remain_size;

		pktcnt = (ep_attr->backup_inep_pktsize +
				ep_attr->ep_pktsize - 1) / ep_attr->ep_pktsize;
	} else {
		ep_attr->backup_inep_pktsize = 0;
		pktcnt = 1;
	}

#ifdef CONFIG_USB_DMA
	writel((pktcnt << 19) | ep_attr->backup_inep_pktsize, DIEP_SIZE(ep_num));

	flush_dcache_range((ulong)ep_attr->pbuffer + ep_attr->receive_size,
			(ulong)ep_attr->pbuffer + ep_attr->receive_size +
			ep_attr->backup_inep_pktsize);

	writel((dma_addr_t)CPHYSADDR(ep_attr->pbuffer + ep_attr->receive_size),
			DIEP_DIEPDMA(ep_num));
#else
	time_out = get_timer(0);
	while ((get_timer(time_out) < udc_dev->data_in_timeout) &&
			!(readl(DIEP_INT(ep_num)) & (1 << 7))) {
	};

	if (readl(DIEP_INT(ep_num)) & (1 << 7)) {
		writel((pktcnt << 19) | ep_attr->backup_inep_pktsize, DIEP_SIZE(ep_num));
	} else
		usberr ("ep %d IN is block and has set NAK (i:%x f:%d)!\n",
				ep_num, readl(DIEP_INT(ep_num)), readl(DIEP_TXFSTS(ep_num)) << 2);
#endif

	if(ep_attr->transfer_size)
		writel(readl(DIEP_EMPMSK) | (1 << ep_num), DIEP_EMPMSK);

	usbdbg("Data_phase pktcnt:%d Ts:%d Tbs:%d Tms:%d Ir:%x\n",
			pktcnt & 0xff, ep_attr->transfer_size , ep_attr->backup_inep_pktsize,
			get_timer(time_out), readl(DIEP_INT(ep_num)));

	set_iep_status(ep_num, EPST_ENABLE, EPST_ACK);
#if 0
	time_out = get_timer(0);

	x1 = readl(DIEP_INT(ep_num));
	x2 = readl(DIEP_CTL(ep_num));
	x3 = readl(DIEP_SIZE(ep_num));
	x4 = readl(DIEP_TXFSTS(ep_num));

	if_6flag () {
//	usberr ("ep %d IN is block and has set NAK (i:%x f:%d t:%d)!\n",
//			ep_num, readl(DIEP_INT(ep_num)), readl(DIEP_TXFSTS(ep_num)) << 2,
//			get_timer(time_out));

		y1 = readl(DOEP_INT(ep_num));
		y2 = readl(DOEP_CTL(ep_num));
		y3 = readl(DOEP_SIZE(ep_num));
		y4 = readl(DOEP_TXFSTS(ep_num));
		//	usbinfo("dsdsds %x %x %x %x %x %x %x %x\n", x1, x2, x3, x4, xx1, xx2, xx3, xx4);
//		usbinfo("asda XXxx %x %x %x %x\n", y1, y2, y3, y4);
	}

//	while (!(readl(DIEP_INT(ep_num)) & 1)) {
//		if(get_timer(time_out) > 100){
//			u32 *data = (u32 *) &udc_dev->ep0_urb->device_request;
//			usbinfo("usb receive setup packet[%x] %x %x\n",
//					(u32)1, data[0], data[1]);
//			xx1 = readl(DIEP_INT(ep_num));
//			xx2 = readl(DIEP_CTL(ep_num));
//			xx3 = readl(DIEP_SIZE(ep_num));
//			xx4 = readl(DIEP_TXFSTS(ep_num));
//			usbinfo("dsdsds %x %x %x %x %x %x %x %x\n", x1, x2, x3, x4, xx1, xx2, xx3, xx4);
//			usbinfo("asda XX %x %x %x %x\n", y1, y2, y3, y4);
//			set_iep_status(ep_num, EPST_ENABLE, EPST_ACK);
//			return;
//		}
//
//	};
#endif
}

static void handle_outep_intr(void)
{
	volatile u32 ep_intr;
	volatile u32 intr;
	volatile u32 ep_num;

	ep_intr = readl(OTG_DAINT) >> 16;
	usbdbg("func: %s %x\n", __func__, ep_intr);

	for(ep_num = 0; ep_num < MAX_ENDPOINTS; ep_num++) {
		if(!(ep_intr & (1 << ep_num)))
			continue;

		intr = readl(DOEP_INT(ep_num));

		usbdbg("##OutEP: %d, intr ox%x##\n", ep_num, intr);

		if (intr & DEP_SETUP_PHASE_DONE) {
			/* SETUP_PHASE_DONE */
			usbdbg("OutEP->setup phase done\n");
#ifdef CONFIG_USB_DMA
			handle_dma_outep_intr(ep_num, DEP_SETUP_PHASE_DONE);
#else
			handle_pio_outep_intr(ep_num, DEP_SETUP_PHASE_DONE, 0);
#endif
			/* Processing setup packet and returns the data */
			handle_setup_packet(ep_num);
			/* Data response phase */
			handle_data_phase(ep_num);
			writel(DEP_SETUP_PHASE_DONE, DOEP_INT(ep_num));
		}

		if (intr & DEP_OUTTOKEN_RECV_EPDIS) {
			usbdbg("OutEP->OUTTknEPdis\n");
			writel(DEP_OUTTOKEN_RECV_EPDIS, DOEP_INT(ep_num));
		}

		if (intr & DEP_STATUS_PHASE_RECV) {
			usbdbg("OutEP->StsPhseRcvd\n");
			writel(DEP_STATUS_PHASE_RECV, DOEP_INT(ep_num));
#ifdef CONFIG_USB_DMA
			handle_dma_outep_intr(ep_num, DEP_STATUS_PHASE_RECV);
#endif
		}

		if (intr & (DEP_NYET_INT | DEP_NAK_INT)) {
			usbdbg("OutEP->Nak\n");
			writel(DEP_NAK_INT | DEP_NYET_INT, DOEP_INT(ep_num));
		}

		if (intr & DEP_XFER_COMP) {
			/* Transfer Completed */
			usbdbg("OutEP->Transfer ok\n");
#ifndef CONFIG_USB_DMA
			set_oep_status(ep_num, EPST_ENABLE, EPST_ACK);
#else
			if (ep_num) {
				void *p;
				struct ep_attribute *ep_attr = &udc_dev->ep_attr[ep_num];
				handle_dma_outep_intr(ep_num, DEP_XFER_COMP);

				if (ep_attr->receive_size)
					invalidate_dcache_range((ulong)ep_attr->pbuffer,
							(ulong)ep_attr->pbuffer + ep_attr->receive_size);

				p = pipe_push_to_dev(udc_dev->base_pipe_dev,
						ep_attr->receive_size);
				if (p) {
					udc_update_out_clannel(ep_num, p,
							udc_dev->ep_attr[ep_num].mxirq_outep_pktsize);
					set_oep_status(ep_num, EPST_ENABLE, EPST_ACK);
				} else {
					udc_dev->data_out_block = 1;
					usberr ("An OUT Transfer Completed but Pipo is block [%p]\n", p);
				}
			} else {
				if (is_quick_enable_ep0())
					set_oep_status(ep_num, EPST_ENABLE, EPST_ACK);
			}
#endif
			writel(DEP_XFER_COMP, DOEP_INT(ep_num));
		}

		if (intr & DEP_AHB_ERR) {
			/* This is generated only in Internal DMA mode when
			 * there is an AHB error during an AHB read/write.
			 */
			usberr("OutEP->AHB Error (AHBErr) !\n");
			writel(DEP_AHB_ERR, DOEP_INT(ep_num));
		}

		/* An interrupt handler to handle an endpoint event */
		return;
	}
}

#ifdef CONFIG_USB_DMA
static void handle_dma_inep_intr(u32 ep_num, u32 status)
{
	struct ep_attribute *ep_attr;
	u32 count;

	ep_attr = &udc_dev->ep_attr[ep_num];
	count = ep_attr->backup_inep_pktsize - (readl(DIEP_SIZE(ep_num))
			& (ep_num ? EPx_PKTMASK : EP0_PKTMASK));

	switch (status) {
	case DEP_XFER_COMP:
		ep_attr->receive_size += count;
		if ((ep_num == 0) && (ep_attr->transfer_size == ep_attr->receive_size)) {
			if (!is_quick_enable_ep0()) {
				set_oep_status(ep_num, EPST_ENABLE, EPST_ACK);
			}
		}
		break;
	case DEP_INTOKEN_RECV_TXFIFO_EMPTY:
		if (ep_attr->transfer_size - ep_attr->receive_size) {
			handle_data_phase(ep_num);
		}
		break;
	}
}

#else
static void handle_pio_inep_intr(u32 ep_num, u32 status)
{
	u32 remain_size;
	struct ep_attribute *ep_attr = &udc_dev->ep_attr[ep_num];

	switch (status) {
	case DEP_XFER_COMP:
		break;
	case DEP_INTOKEN_RECV_TXFIFO_EMPTY:
		break;
	case DEP_TXFIFO_EMPTY:
		remain_size = ep_attr->transfer_size - ep_attr->receive_size;
		if (remain_size) {
			u32 time_out;

			if (remain_size > ep_attr->ep_pktsize)
				ep_attr->backup_inep_pktsize = ep_attr->ep_pktsize;
			else
				ep_attr->backup_inep_pktsize = remain_size;

			otg_pio_write_fifo(ep_num, (void *)((u32)ep_attr->pbuffer +
						ep_attr->receive_size), ep_attr->backup_inep_pktsize);

			time_out = get_timer(0);

			while ((get_timer(time_out) < udc_dev->data_in_timeout) &&
					!(readl(DIEP_INT(ep_num)) & (1 << 7))) {
			};

			if (readl(DIEP_INT(ep_num)) & (1 << 7))
				ep_attr->receive_size += ep_attr->backup_inep_pktsize;
		}
		break;
	}
}
#endif

static void handle_inep_intr(void)
{
	volatile u32 ep_intr;
	volatile u32 intr;
	volatile u32 ep_num;

	usbdbg("func: %s\n", __func__);

	for(ep_num = 0; ep_num < MAX_ENDPOINTS; ep_num++) {

		ep_intr = readl(OTG_DAINT) & 0xffff;

		if(!(ep_intr & (1 << ep_num)))
			continue;

		intr = readl(DIEP_INT(ep_num));
		usbdbg("##InEP: %d, intr ox%x##\n", ep_num, intr);

		if (intr & DEP_XFER_COMP) {
			/* Transfer Completed Interrupt */
			usbdbg("InEP->tr completed %d\n", ep_num);
#ifdef CONFIG_USB_DMA
			handle_dma_inep_intr(ep_num, DEP_XFER_COMP);
#else
			handle_pio_inep_intr(ep_num, DEP_XFER_COMP);
#endif
			otg_flush_tx_fifo(0x10);
			writel(0, DIEP_EMPMSK);
			writel(DEP_XFER_COMP, DIEP_INT(ep_num));
		}

		if (intr & DEP_EPDIS_INT) {
			/* Endpoint Disabled Interrupt */
			usbdbg("InEP->ep disable\n");
			writel(DEP_EPDIS_INT, DIEP_INT(ep_num));
		}

		if (intr & DEP_TIME_OUT) {
			/* Timeout Condition */
			usberr("InEP->time out\n");
			writel(DEP_TIME_OUT, DIEP_INT(ep_num));
		}

		if (intr & DEP_INTOKEN_RECV_TXFIFO_EMPTY) {
			/* intoken recv while txfifo empty */
			usbdbg("InEP->intoken txfifo empty\n");
#ifdef CONFIG_USB_DMA
			handle_dma_inep_intr(ep_num, DEP_INTOKEN_RECV_TXFIFO_EMPTY);
#endif
			writel(DEP_INTOKEN_RECV_TXFIFO_EMPTY, DIEP_INT(ep_num));
		}

		if (intr & DEP_TXFIFO_EMPTY) {
			/* tx fifo empty interrupt */
			usbdbg("InEP->txfifo empty %x\n", readl(DIEP_INT(ep_num)));
#ifndef CONFIG_USB_DMA
			handle_pio_inep_intr(ep_num, DEP_TXFIFO_EMPTY);
#endif
			writel(DEP_TXFIFO_EMPTY, DIEP_INT(ep_num));
		}

		if (intr & DEP_NAK_INT) {
			/* NAK interrupt */
			usbdbg("InEP->NAK %x\n", readl(DIEP_INT(ep_num)));
			writel(DEP_NAK_INT, DIEP_INT(ep_num));
		}

		if (intr & DEP_AHB_ERR) {
			/* This is generated only in Internal DMA mode when
			 * there is an AHB error during an AHB read/write.
			 */
			usberr("InEP->AHB Error (AHBErr) !\n");
			writel(DEP_AHB_ERR, DIEP_INT(ep_num));
		}
	}
}

#ifndef CONFIG_USB_DMA
static void handle_rxfifo_nempty(void)
{
	u32 ep_num;
	u32 count;
	u32 rxstsp;

	rxstsp = readl(&otg_reg->grxstsp);
	ep_num = (rxstsp >> 0) & 0xf;

	usbdbg ("##rxfifo st: type ox%x pid ox%x count ox%x ep ox%x##\n",
			((rxstsp >> 17) & 0xf),
			((rxstsp >> 15) & 3),
			((rxstsp >> 4) & 0x7ff),
			((rxstsp >> 0) & 0xf)
		  );

	usbdbg("rxnp->");
	count = (rxstsp >> GRXSTSP_BYTE_CNT_BIT) & GRXSTSP_BYTE_CNT_MASK;

	switch(rxstsp & GRXSTSP_PKSTS_MASK) {
	case GRXSTSP_PKSTS_GOUT_NAK:
		/* Global OUT NAK */
		usbmsg("Nak\n");
		break;
	case GRXSTSP_PKSTS_GOUT_RECV:
		/* OUT data packet received */
		usbmsg("Recv %d\n", count);
		handle_pio_outep_intr(ep_num, rxstsp & GRXSTSP_PKSTS_MASK, rxstsp);
		break;
	case GRXSTSP_PKSTS_TX_COMP:
		/* OUT transfer completed */
		usbmsg("Completed\n");
		break;
	case GRXSTSP_PKSTS_SETUP_COMP:
		/* SETUP transaction completed */
		usbmsg("s_Completed\n");
		break;
	case GRXSTSP_PKSTS_SETUP_RECV:
		/* SETUP data packet received */
		usbmsg("s_Recv %d\n", count);
		handle_pio_outep_intr(ep_num, rxstsp & GRXSTSP_PKSTS_MASK, rxstsp);
		break;
	default:
		usberr("Unknow %x %d!!!\n", rxstsp & GRXSTSP_PKSTS_MASK, count);
		break;
	}
}
#endif

static void otg_phy_irq(void)
{
	volatile u32 otg_status;

	otg_status = readl(&otg_reg->gintsts);

	/* In Device mode, in the core sets this bit to indicate that
	 *  an SOF token has been received on the USB.
	 */
	if(!(otg_status & (GINTSTS_USB_EARLYSUSPEND |
			GINTSTS_USB_SUSPEND | GINTSTS_USB_RESET))
			&& (!(otg_status & 0x8))) {
		if (otg_status & 0xf8000000)
			writel(otg_status & 0xf8000000, &otg_reg->gintsts);
		return;
	}

#ifdef DEBUG
	if(otg_status & (
			GINTSTS_USB_EARLYSUSPEND |
			GINTSTS_IEP_INTR |
			GINTSTS_OEP_INTR |
			GINTSTS_RXFIFO_NEMPTY |
			GINTSTS_ENUM_DONE |
			GINTSTS_USB_RESET)) {
		usbdbg(">>>");
		usbdbg("ox%x %x\n", otg_status, readl(OTG_DAINT));
	}
#endif

	/* USB_EARLYSUSPEND */
	if (otg_get_status() & GINTSTS_USB_EARLYSUSPEND) {
		handle_early_suspend_intr();
		return;
	}

	/* USB_EARLYSUSPEND */
	if (otg_get_status() & GINTSTS_USB_SUSPEND) {
		handle_suspend_intr();
		return;
	}

	/* Start of (micro)Frame (Sof) */
	if (otg_get_status() & GINTSTS_START_FRAM) {
		handle_start_frame_intr();
	}

	/* reset interrupt handle */
	if (otg_get_status() & GINTSTS_USB_RESET) {
		handle_reset_intr();
		return;
	}

	/* enum done */
	if (otg_get_status() & GINTSTS_ENUM_DONE) {
		handle_enum_done_intr();
		return;
	}

	/* IN Endpoints Interrupt */
	if (otg_get_status() & GINTSTS_IEP_INTR) {
		handle_inep_intr();
		return;
	}

#ifndef CONFIG_USB_DMA
	/* RXFIFO_NEMPTY */
	if (otg_get_status() & GINTSTS_RXFIFO_NEMPTY) {
		handle_rxfifo_nempty();
		return;
	}
#endif

	/* OUT Endpoints Interrupt */
	if (otg_get_status() & GINTSTS_OEP_INTR) {
		handle_outep_intr();
		return;
	}
}

int udc_high_speed(void)
{
	return (readl(OTG_DSTS) & (3 << 1)) ? 0 : 1;
}

int udc_func_register(struct urb *ep0_urb)
{
	struct usb_endpoint_instance *ep;

	if (!ep0_urb) {
		usberr("ep0_urb no defined, udc func register error\n");
		return -EINVAL;
	}

	if (!udc_dev) {
		usberr("udc_dev defined fail, udc func register error\n");
		return -EIO;
	}

	if (udc_dev->ep0_urb) {
		usberr("udc function has registered\n");
		return -EEXIST;
	}

#ifdef CONFIG_USB_DMA
	if ((u32)ep0_urb->buffer & (CONFIG_SYS_CACHELINE_SIZE - 1)) {
		usberr("ep0_urb buffer must be %d aligned in DMA mode\n",
				CONFIG_SYS_CACHELINE_SIZE);
		return -EINVAL;
	}
#endif

	udc_dev->ep0_urb = ep0_urb;
	ep = &ep0_urb->device->bus->endpoint_array[0];
	ep->rcv_urb = ep->tx_urb = ep0_urb;

	udc_dev->ep_attr[0].pbuffer = ep0_urb->buffer;
	udc_dev->data_out_epnum = 3;
	udc_dev->data_out_block = 0;
	udc_dev->data_in_epnum = 2;
	udc_dev->data_in_timeout = TRANSFER_TIME_OUT;

	return 0;
}

static int udc_data_transfer(struct udc_pipe_dev *pipe_dev,
		void *p, unsigned int size)
{
	struct ep_attribute *ep_attr;
	u32 time_out;
	u32 tmp;

	usbdbg("%s %d\n", __func__, size);
	ep_attr = &udc_dev->ep_attr[udc_dev->data_in_epnum];
	ep_attr->receive_size = 0;
	ep_attr->transfer_size = size;
	ep_attr->pbuffer = p;
	tmp = ep_attr->receive_size;
	handle_data_phase(udc_dev->data_in_epnum);

	time_out = get_timer(0);
	do {
		otg_phy_irq();

		if (tmp < ep_attr->receive_size) {
			tmp = ep_attr->receive_size;
			time_out = get_timer(0);
		} else if (get_timer(time_out) >= udc_dev->data_in_timeout) {
			if (udc_dev->data_in_echo)
				usb_print(".");
			set_iep_status(udc_dev->data_in_epnum, EPST_DISABLE, EPST_NAK);
			otg_flush_tx_fifo(0x10);
			return -ETIME;
		}
	} while (ep_attr->receive_size != size);

	return size;
}

static int udc_pipe_ioctl(struct udc_pipe_dev *pipe_dev,
		unsigned int cmd, unsigned long arg)
{
	void *p = NULL;

	if (!pipe_dev) {
		usberr("pipe_dev no defined, udc pipe register error\n");
		return -EINVAL;
	}

	switch (cmd) {
	case UDC_IOCTL_SET_CURRENT:
		udc_dev->backup_pipe_dev = udc_dev->base_pipe_dev;
	case UDC_IOCTL_SET_BYPASS:
		if (udc_dev->backup_pipe_dev != udc_dev->base_pipe_dev) {
			udc_dev->base_pipe_dev = udc_dev->backup_pipe_dev;
			pipe_dev = udc_dev->base_pipe_dev;
		}
	case UDC_IOCTL_SET_BASE:
		udc_dev->base_pipe_dev = pipe_dev;
		p = pipe_push_to_dev(pipe_dev, 0);
		if (!p || !udc_dev->ep_attr[udc_dev->data_out_epnum].ep_pktsize)
			break;
		udc_update_out_clannel(udc_dev->data_out_epnum, p,
				udc_dev->ep_attr[udc_dev->data_out_epnum].mxirq_outep_pktsize);
		if (udc_dev->data_out_block) {
			udc_dev->data_out_block = 0;
			set_oep_status(udc_dev->data_out_epnum, EPST_ENABLE, EPST_ACK);
		}
		break;
	case UDC_IOCTL_SET_TIMEOUT:
		udc_dev->data_in_timeout = arg;
		break;
#ifdef CONFIG_USB_DMA
	case UDC_IOCTL_BIG_PACKET:
		if (!arg || (arg & 0x1ff) || (arg > EPx_DMA_PKTSIZE)) {
			usberr ("the packet size %d illegal request !\n", (u32)arg);
		} else {
			udc_dev->ep_attr[udc_dev->data_out_epnum].mxirq_outep_pktsize = arg;
		}
		break;
	case UDC_IOCTL_NORMAL_PACKET:
		udc_dev->ep_attr[udc_dev->data_out_epnum].mxirq_outep_pktsize =
			udc_dev->ep_attr[udc_dev->data_out_epnum].ep_pktsize;
		break;
#else
	case UDC_IOCTL_BIG_PACKET:
	case UDC_IOCTL_NORMAL_PACKET:
		usbdbg("Pio mode cannot use BIG_PACKET mode (ox%x)\n", cmd);
		break;
#endif
	default:
		usberr("ioctl unknown command ox%x\n", cmd);
		break;
	}

	return 0;
}

int udc_pipe_register(struct udc_pipe_dev *pipe_dev)
{
	if (!pipe_dev) {
		usberr("pipe_dev no defined, udc pipe register error\n");
		return -EINVAL;
	}

	if (!pipe_dev->f_dev_push_buffer) {
		usberr("dev_push_buffer function no defined,"
				" udc pipe register error\n");
		return -EINVAL;
	}

	list_add_tail(&pipe_dev->list, &udc_dev->pipe_list);

	pipe_dev->f_udc_ioctl = udc_pipe_ioctl;
	pipe_dev->f_udc_push_buffer = udc_data_transfer;

	return 0;
}

void udc_process(void)
{
	/* otg_phy_irq poll checkout otg phy interrupt status
	 * and report usb event */
	otg_phy_irq();
}

int pipe_push_to_udc(struct udc_pipe_dev *pipe_dev,
		void *b, unsigned int size)
{
	return pipe_dev->f_udc_push_buffer(pipe_dev, b, size);
}

void *pipe_push_to_dev(struct udc_pipe_dev *pipe_dev, unsigned int size)
{
	void *p;
	p = pipe_dev->f_dev_push_buffer(pipe_dev, size);
#ifdef CONFIG_USB_DMA
	if ((u32)p & (CONFIG_SYS_CACHELINE_SIZE - 1)) {
		usberr ("Return buffer address Aligned with %d in DMA mode\n",
				CONFIG_SYS_CACHELINE_SIZE);
		return NULL;
	}
#endif
	return p;
}

int pipe_ioctl_udc(struct udc_pipe_dev *pipe_dev,
		unsigned int cmd, unsigned long arg)
{
	return pipe_dev->f_udc_ioctl(pipe_dev, cmd, arg);
}

static int cmd_usb(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc < 2)
		return CMD_RET_USAGE;

	if (strcmp(argv[1], "info") == 0) {
		struct list_head *pos;
		struct udc_pipe_dev *dev;

		if (argc != 2)
			return CMD_RET_USAGE;

		usb_print ("usb speed %s, ep0 packet size %d, IN transfer time out Warn %s\n",
				udc_high_speed() ? "HIHG" : "FULL", udc_dev->ep_attr[0].ep_pktsize,
				udc_dev->data_in_echo ? "ON" : "OFF");

		usb_print ("usb register pipe device:\n\t");

		list_for_each(pos, &udc_dev->pipe_list) {
			dev = list_entry(pos, struct udc_pipe_dev, list);
			usb_print ("%s ", dev->name);
		}

		usb_print ("\n");
		return CMD_RET_SUCCESS;
	} else if (strncmp(argv[1], "echo", 4) == 0) {
		u32 flag;

		if (argc != 3)
			return CMD_RET_USAGE;

		usb_print ("In transfer time out warning ");
		flag = simple_strtoul(argv[2], NULL, 10);
		if (flag) {
			usb_print ("ON\n");
			udc_dev->data_in_echo = 1;
		} else {
			usb_print ("OFF\n");
			udc_dev->data_in_echo = 0;
		}

		return CMD_RET_SUCCESS;
	}

	return CMD_RET_FAILURE;
}

U_BOOT_CMD(
	usb, 6, 1, cmd_usb,
	"usb function command",
	"info - usb devices information\n"
	"usb echo [0/1]- In transfer time out flag echo [1:ON 0:OFF]"
	);

