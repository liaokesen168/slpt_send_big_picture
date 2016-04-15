/*
 * Copyright (C) 2013 Ingenic Semiconductor Co., Ltd.
 * Authors: Kage Shen <kkshen@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 3 of the License, or (at your option) any later version.
 */

#ifndef __M200_UDC_REG_H__
#define __M200_UDC_REG_H__


/**
 * This union represents the bit fields in the DMA Descriptor
 * status quadlet. Read the quadlet into the <i>d32</i> member then
 * set/clear the bits using the <i>b</i>it, <i>b_iso_out</i> and
 * <i>b_iso_in</i> elements.
 */
typedef union dev_dma_desc_sts {
		/** raw register data */
	uint32_t d32;
		/** quadlet bits */
	struct {
		/** Received number of bytes */
		unsigned bytes:16;
		/** NAK bit - only for OUT EPs */
		unsigned nak:1;
		unsigned reserved17_22:6;
		/** Multiple Transfer - only for OUT EPs */
		unsigned mtrf:1;
		/** Setup Packet received - only for OUT EPs */
		unsigned sr:1;
		/** Interrupt On Complete */
		unsigned ioc:1;
		/** Short Packet */
		unsigned sp:1;
		/** Last */
		unsigned l:1;
		/** Receive Status */
		unsigned sts:2;
		/** Buffer Status */
		unsigned bs:2;
	} b;

		/** iso out quadlet bits */
	struct {
		/** Received number of bytes */
		unsigned rxbytes:11;

		unsigned reserved11:1;
		/** Frame Number */
		unsigned framenum:11;
		/** Received ISO Data PID */
		unsigned pid:2;
		/** Interrupt On Complete */
		unsigned ioc:1;
		/** Short Packet */
		unsigned sp:1;
		/** Last */
		unsigned l:1;
		/** Receive Status */
		unsigned rxsts:2;
		/** Buffer Status */
		unsigned bs:2;
	} b_iso_out;

		/** iso in quadlet bits */
	struct {
		/** Transmited number of bytes */
		unsigned txbytes:12;
		/** Frame Number */
		unsigned framenum:11;
		/** Transmited ISO Data PID */
		unsigned pid:2;
		/** Interrupt On Complete */
		unsigned ioc:1;
		/** Short Packet */
		unsigned sp:1;
		/** Last */
		unsigned l:1;
		/** Transmit Status */
		unsigned txsts:2;
		/** Buffer Status */
		unsigned bs:2;
	} b_iso_in;
} dev_dma_desc_sts_t;

/**
 * DMA Descriptor structure
 *
 * DMA Descriptor structure contains two quadlets:
 * Status quadlet and Data buffer pointer.
 */
typedef struct dwc_otg_dev_dma_desc {
	/** DMA Descriptor status quadlet */
	dev_dma_desc_sts_t status;
	/** DMA Descriptor data buffer pointer */
	uint32_t buf;
} dwc_otg_dev_dma_desc_t;

/**
 * This union represents the bit fields in the DMA Descriptor
 * status quadlet for host mode. Read the quadlet into the <i>d32</i> member then
 * set/clear the bits using the <i>b</i>it elements.
 */
typedef union host_dma_desc_sts {
	/** raw register data */
	uint32_t d32;
	/** quadlet bits */

	/* for non-isochronous  */
	struct {
		/** Number of bytes */
		unsigned n_bytes:17;
		/** QTD offset to jump when Short Packet received - only for IN EPs */
		unsigned qtd_offset:6;
		/**
		 * Set to request the core to jump to alternate QTD if
		 * Short Packet received - only for IN EPs
		 */
		unsigned a_qtd:1;
		 /**
		  * Setup Packet bit. When set indicates that buffer contains
		  * setup packet.
		  */
		unsigned sup:1;
		/** Interrupt On Complete */
		unsigned ioc:1;
		/** End of List */
		unsigned eol:1;
		unsigned reserved27:1;
		/** Rx/Tx Status */
		unsigned sts:2;
#define DMA_DESC_STS_PKTERR	1
		unsigned reserved30:1;
		/** Active Bit */
		unsigned a:1;
	} b;
	/* for isochronous */
	struct {
		/** Number of bytes */
		unsigned n_bytes:12;
		unsigned reserved12_24:13;
		/** Interrupt On Complete */
		unsigned ioc:1;
		unsigned reserved26_27:2;
		/** Rx/Tx Status */
		unsigned sts:2;
		unsigned reserved30:1;
		/** Active Bit */
		unsigned a:1;
	} b_isoc;
} host_dma_desc_sts_t;

/****************************************************************************/
/** DWC_otg Core registers .
 * The dwc_otg_core_global_regs structure defines the size
 * and relative field offsets for the Core Global registers.
 */
typedef struct dwc_otg_core_global_regs {
	/** OTG Control and Status Register.  <i>Offset: 000h</i> */
	volatile uint32_t gotgctl;
	/** OTG Interrupt Register.           <i>Offset: 004h</i> */
	volatile uint32_t gotgint;
	/**Core AHB Configuration Register.   <i>Offset: 008h</i> */
	volatile uint32_t gahbcfg;

	/**Core USB Configuration Register.   <i>Offset: 00Ch</i> */
	volatile uint32_t gusbcfg;
	/**Core Reset Register.               <i>Offset: 010h</i> */
	volatile uint32_t grstctl;
	/**Core Interrupt Register.           <i>Offset: 014h</i> */
	volatile uint32_t gintsts;
	/**Core Interrupt Mask Register.      <i>Offset: 018h</i> */
	volatile uint32_t gintmsk;
	/**Receive Status Queue Read Register (Read Only).  <i>Offset: 01Ch</i> */
	volatile uint32_t grxstsr;
	/**Receive Status Queue Read & POP Register (Read Only).  <i>Offset: 020h</i> */
	volatile uint32_t grxstsp;
	/**Receive FIFO Size Register.        <i>Offset: 024h</i> */
	volatile uint32_t grxfsiz;
	/**Non Periodic Transmit FIFO Size Register.        <i>Offset: 028h</i> */
	volatile uint32_t gnptxfsiz;
	/**Non Periodic Transmit FIFO/Queue Status Register (Read Only). <i>Offset: 02Ch</i> */
	volatile uint32_t gnptxsts;
	/**I2C Access Register.               <i>Offset: 030h</i> */
	volatile uint32_t gi2cctl;
	/**PHY Vendor Control Register.       <i>Offset: 034h</i> */
	volatile uint32_t gpvndctl;
	/**General Purpose Input/Output Register. <i>Offset: 038h</i> */
	volatile uint32_t ggpio;
	/**User ID Register.                      <i>Offset: 03Ch</i> */
	volatile uint32_t guid;
	/**Synopsys ID Register (Read Only).      <i>Offset: 040h</i> */
	volatile uint32_t gsnpsid;
	/**User HW Config1 Register (Read Only).  <i>Offset: 044h</i> */
	volatile uint32_t ghwcfg1;
	/**User HW Config2 Register (Read Only).  <i>Offset: 048h</i> */
	volatile uint32_t ghwcfg2;

	/**User HW Config3 Register (Read Only).  <i>Offset: 04Ch</i> */
	volatile uint32_t ghwcfg3;
	/**User HW Config4 Register (Read Only).  <i>Offset: 050h</i> */
	volatile uint32_t ghwcfg4;
	/**core lpm configuration register        <i>Offset: 054h</i> */
	volatile uint32_t glpmcfg;
	/**power down register                    <i>Offset: 058h</i> */
	volatile uint32_t gpwrdn;
	/**dfifo software config register         <i>Offset: 05ch</i> */
	volatile uint32_t gdfifocfg;
	/**adp timer, control and status register <i>Offset: 060h</i> */
	volatile uint32_t gadpctl;
	/** Reserved                              <i>Offset: 064h-0FFh</i> */
	volatile uint32_t reserved39[39];
	/** Host Periodic Transmit FIFO Size Register. <i>Offset: 100h</i> */
	volatile uint32_t hptxfsiz;
	/** Device Periodic Transmit FIFO#n Register if dedicated fifos are disabled,
		otherwise Device Transmit FIFO#n Register.
	 * <i>Offset: 104h + (FIFO_Number-1)*04h, 1 <= FIFO Number <= 15 (1<=n<=15).</i> */
	volatile uint32_t dtxfsiz[15];
} dwc_otg_core_global_regs_t;

/**
 * Device Logical IN Endpoint-Specific Registers. <i>Offsets
 * 900h-AFCh</i>
 *
 * There will be one set of endpoint registers per logical endpoint
 * implemented.
 *
 * <i>These registers are visible only in Device mode and must not be
 * accessed in Host mode, as the results are unknown.</i>
 */
typedef struct dwc_otg_dev_in_ep_regs {
	/** Device IN Endpoint Control Register. <i>Offset:900h +
	 * (ep_num * 20h) + 00h</i> */
	volatile uint32_t diepctl;
	/** Reserved. <i>Offset:900h + (ep_num * 20h) + 04h</i> */
	uint32_t reserved04;
	/** Device IN Endpoint Interrupt Register. <i>Offset:900h +
	 * (ep_num * 20h) + 08h</i> */
	volatile uint32_t diepint;
	/** Reserved. <i>Offset:900h + (ep_num * 20h) + 0Ch</i> */
	uint32_t reserved0C;
	/** Device IN Endpoint Transfer Size
	 * Register. <i>Offset:900h + (ep_num * 20h) + 10h</i> */
	volatile uint32_t dieptsiz;
	/** Device IN Endpoint DMA Address Register. <i>Offset:900h +
	 * (ep_num * 20h) + 14h</i> */
	volatile uint32_t diepdma;
	/** Device IN Endpoint Transmit FIFO Status Register. <i>Offset:900h +
	 * (ep_num * 20h) + 18h</i> */
	volatile uint32_t dtxfsts;
	/** Device IN Endpoint DMA Buffer Register. <i>Offset:900h +
	 * (ep_num * 20h) + 1Ch</i> */
	volatile uint32_t diepdmab;
} dwc_otg_dev_in_ep_regs_t;

/**
//
 * B00h-CFCh</i>
 *
 * There will be one set of endpoint registers per logical endpoint
 * implemented.
 *
 * <i>These registers are visible only in Device mode and must not be
 * accessed in Host mode, as the results are unknown.</i>
 */
typedef struct dwc_otg_dev_out_ep_regs {
	/** Device OUT Endpoint Control Register. <i>Offset:B00h +
	 * (ep_num * 20h) + 00h</i> */
	volatile uint32_t doepctl;
	/** Reserved. <i>Offset:B00h + (ep_num * 20h) + 04h</i> */
	uint32_t reserved04;
	/** Device OUT Endpoint Interrupt Register. <i>Offset:B00h +
	 * (ep_num * 20h) + 08h</i> */
	volatile uint32_t doepint;
	/** Reserved. <i>Offset:B00h + (ep_num * 20h) + 0Ch</i> */
	uint32_t reserved0C;
	/** Device OUT Endpoint Transfer Size Register. <i>Offset:
	 * B00h + (ep_num * 20h) + 10h</i> */
	volatile uint32_t doeptsiz;
	/** Device OUT Endpoint DMA Address Register. <i>Offset:B00h
	 * + (ep_num * 20h) + 14h</i> */
	volatile uint32_t doepdma;
	/** Reserved. <i>Offset:B00h + 	 * (ep_num * 20h) + 18h</i> */
	uint32_t unused;
	/** Device OUT Endpoint DMA Buffer Register. <i>Offset:B00h
	 * + (ep_num * 20h) + 1Ch</i> */
	uint32_t doepdmab;
} dwc_otg_dev_out_ep_regs_t;

#define OTG_BASE    0xb3500000

#define GDTXFIFO_SIZE  (OTG_BASE + 0x104)

#define EP0_FIFO       (OTG_BASE + 0x1000)
#define EP1_FIFO       (OTG_BASE + 0x2000)

#define EP_FIFO(n)     (OTG_BASE + (n+1)*0x1000) // FiX ME

#define OTG_DCFG       (OTG_BASE + 0x800)
#define OTG_DCTL       (OTG_BASE + 0x804)
#define OTG_DSTS       (OTG_BASE + 0x808)
#define DIEP_MASK      (OTG_BASE + 0x810)
#define DOEP_MASK      (OTG_BASE + 0x814)
#define OTG_DAINT      (OTG_BASE + 0x818)
#define DAINT_MASK     (OTG_BASE + 0x81c)

#define DIEP_EMPMSK    (OTG_BASE + 0x834)

#define DIEP_CTL(n)    (OTG_BASE + (0x900 + (n)*0x20))
#define DOEP_CTL(n)    (OTG_BASE + (0xb00 + (n)*0x20))

#define DIEP_INT(n)    (OTG_BASE + (0x908 + (n)*0x20))
#define DOEP_INT(n)    (OTG_BASE + (0xb08 + (n)*0x20))

#define DIEP_SIZE(n)   (OTG_BASE + (0x910 + (n)*0x20))
#define DOEP_SIZE(n)   (OTG_BASE + (0xb10 + (n)*0x20))

#define DIEP_TXFSTS(n) (OTG_BASE + (0x918 + (n)*0x20))
#define DOEP_TXFSTS(n) (OTG_BASE + (0xb18 + (n)*0x20))

#define DIEP_DIEPDMA(n)   (OTG_BASE + (0x914 + (n)*0x20))
#define DOEP_DIEPDMA(n)   (OTG_BASE + (0xb14 + (n)*0x20))

#define DIEP_DIEPDMAB(n)   (OTG_BASE + (0x91c + (n)*0x20))
#define DOEP_DIEPDMAB(n)   (OTG_BASE + (0xb1c + (n)*0x20))

#define USBRDT_VBFIL_LD_EN      25
#define USBPCR_TXPREEMPHTUNE    6
#define USBPCR_POR              (1 << 22)
#define USBPCR_OTG_MODE         (1 << 31)
#define USBPCR_COMMONONN        (1 << 25)
#define USBPCR_VBUSVLDEXT       (1 << 24)
#define USBPCR_VBUSVLDEXTSEL    (1 << 23)
#define USBPCR_OTG_DISABLE      (1 << 20)
#define USBPCR_IDPULLUP_MASK    (3 << 28)

#define OPCR_SPENDN0            7

#define USBPCR1_USB_SEL         (1 << 28)
#define USBPCR1_WORD_IF0        (1 << 19)
#define USBPCR1_WORD_IF1        (1 << 18)
#define USBPCR1_REFCLK_DEV      24
#define USBPCR1_REFCLK_MASK     (3 << USBPCR1_REFCLK_DEV)

#define USBRDT_VBFIL_EN         (1 << 25)

#define GUSBCFG_PHY_16BIT       (1 << 3)
#define GUSBCFG_TRDTIME_9       (9 << 10)
#define GUSBCFG_TRDTIME_6       (6 << 10)

/* GRSTCTL */
#define GRSTCTL_AHB_IDLE        (1 << 31)
#define GRSTCTL_TXFNUM_ALL      (0x10 << 6)
#define GRSTCTL_TXFIFO_FLUSH    (1 << 5)
#define GRSTCTL_RXFIFO_FLUSH    (1 << 4)
#define GRSTCTL_INTK_FLUSH      (1 << 3)
#define GRSTCTL_FRMCNT_RST      (1 << 2)
#define GRSTCTL_CORE_RST        (1 << 0)

/* DIOEPCTL */
#define DEP_EP0_MAXPKET_SIZE    64
#define DEP_EP0_MPS_64          (0x0)
#define DEP_EP0_MPS_32          (0x1)
#define DEP_EP0_MPS_16          (0x2)
#define DEP_EP0_MPS_8           (0x3)

#define DEP_ENA_BIT         (1 << 31)
#define DEP_DISENA_BIT      (1 << 30)
#define DEP_SET_NAK         (1 << 27)
#define DEP_CLEAR_NAK       (1 << 26)
#define DEP_TYPE_MASK       (0x3 << 18)
#define DEP_TYPE_CNTL       (0x0 << 18)
#define DEP_TYPE_ISO        (0x1 << 18
#define DEP_TYPE_BULK       (0x2 << 18)
#define DEP_TYPE_INTR       (0x3 << 18)
#define USB_ACTIVE_EP       (1 << 15)

/* DIOEPINT */
#define DEP_NYET_INT         (1 << 14)
#define DEP_NAK_INT         (1 << 13)
#define DEP_BABBLE_ERR_INT  (1 << 12)
#define DEP_PKT_DROP_STATUS (1 << 11)
#define DEP_BNA_INT         (1 << 9)
#define DEP_TXFIFO_UNDRN    (1 << 8)
#define DEP_OUTPKT_ERR      (1 << 8)
#define DEP_TXFIFO_EMPTY    (1 << 7)
#define DEP_INEP_NAKEFF     (1 << 6)
#define DEP_B2B_SETUP_RECV  (1 << 6)
#define DEP_INTOKEN_EPMISATCH          (1 << 5)
#define DEP_STATUS_PHASE_RECV          (1 << 5)
#define DEP_INTOKEN_RECV_TXFIFO_EMPTY  (1 << 4)
#define DEP_OUTTOKEN_RECV_EPDIS        (1 << 4)
#define DEP_TIME_OUT                   (1 << 3)
#define DEP_SETUP_PHASE_DONE           (1 << 3)
#define DEP_AHB_ERR         (1 << 2)
#define DEP_EPDIS_INT       (1 << 1)
#define DEP_XFER_COMP       (1 << 0)

/* DOEPSIZ0 */
#define DOEPSIZE0_SUPCNT_1    (0x1 << 29)
#define DOEPSIZE0_SUPCNT_2    (0x2 << 29)
#define DOEPSIZE0_SUPCNT_3    (0x3 << 29)
#define DOEPSIZE0_PKTCNT_BIT  (1 << 19)

/* GINTSTS */
#define GINTSTS_RSUME_DETE      (1 << 31)
#define GINTSTS_CONID_STSCHG    (1 << 28)
#define GINTSTS_RESET_DETE      (1 << 23)
#define GINTSTS_FETCH_SUSPEND   (1 << 22)
#define GINTSTS_OEP_INTR        (1 << 19)
#define GINTSTS_IEP_INTR        (1 << 18)
#define GINTSTS_EP_MISMATCH     (1 << 17)
#define GINTSTS_ENUM_DONE       (1 << 13)
#define GINTSTS_USB_RESET       (1 << 12)
#define GINTSTS_USB_SUSPEND     (1 << 11)
#define GINTSTS_USB_EARLYSUSPEND   (1 << 10)
#define GINTSTS_I2C_INT         (1 << 9)
#define GINTSTS_ULPK_CKINT      (1 << 8)
#define GINTSTS_GOUTNAK_EFF     (1 << 7)
#define GINTSTS_GINNAK_EFF      (1 << 6)
#define GINTSTS_NPTXFIFO_EMPTY  (1 << 5)
#define GINTSTS_RXFIFO_NEMPTY   (1 << 4)
#define GINTSTS_START_FRAM      (1 << 3)
#define GINTSTS_OTG_INTR        (1 << 2)
#define GINTSTS_MODE_MISMATCH   (1 << 1)

/* GRXSTSR/GRXSTSP */
#define GRXSTSP_PKSTS_MASK          (0xf << 17)
#define GRXSTSP_PKSTS_GOUT_NAK      (0x1 << 17)
#define GRXSTSP_PKSTS_GOUT_RECV     (0x2 << 17)
#define GRXSTSP_PKSTS_TX_COMP       (0x3 << 17)
#define GRXSTSP_PKSTS_SETUP_COMP    (0x4 << 17)
#define GRXSTSP_PKSTS_SETUP_RECV    (0x6 << 17)
#define GRXSTSP_BYTE_CNT_MASK       (0x7ff)
#define GRXSTSP_BYTE_CNT_BIT        4
#define GRXSTSP_EPNUM_MASK          (0xf)
#define GRXSTSP_EPNUM_BIT           (1 << 0)


/* DCTL */
#define DCTL_CLR_GOUTNAK    (1 << 10)
#define DCTL_SET_GOUTNAK    (1 << 9)
#define DCTL_CLR_GNPINNAK   (1 << 8)
#define DCTL_SET_GNPINNAK   (1 << 7)
#define DCTL_SOFT_DISCONN   (1 << 1)

/* DSTS */
#define DSTS_ENUM_SPEED_MASK    (0x3 << 1)
#define DSTS_ENUM_SPEED_BIT     (1 << 1)
#define DSTS_ENUM_SPEED_HIGH    (0x0 << 1)
#define DSTS_ENUM_SPEED_FULL_30OR60     (0x1 << 1)
#define DSTS_ENUM_SPEED_LOW     (0x2 << 1)
#define DSTS_ENUM_SPEED_FULL_48 (0x3 << 1)

/*
 * Standard requests
 */
#define USB_REQ_GET_STATUS          0x00
#define USB_REQ_CLEAR_FEATURE       0x01
/* 0x02 is reserved */
#define USB_REQ_SET_FEATURE         0x03
/* 0x04 is reserved */
#define USB_REQ_SET_ADDRESS         0x05
#define USB_REQ_GET_DESCRIPTOR      0x06
#define USB_REQ_SET_DESCRIPTOR      0x07
#define USB_REQ_GET_CONFIGURATION   0x08
#define USB_REQ_SET_CONFIGURATION   0x09
#define USB_REQ_GET_INTERFACE       0x0A
#define USB_REQ_SET_INTERFACE       0x0B
#define USB_REQ_SYNCH_FRAME         0x0C

/* Vendor requests. */
#define EP0_GET_CPU_INFO            0x00
#define EP0_SET_DATA_ADDRESS        0x01
#define EP0_SET_DATA_LENGTH     0x02
#define EP0_FLUSH_CACHES        0x03
#define EP0_PROG_START1         0x04
#define EP0_PROG_START2         0x05

/* Descriptor types ... USB 2.0 spec table 9.5 */
#define USB_DT_DEVICE           0x01
#define USB_DT_CONFIG           0x02
#define USB_DT_STRING           0x03
#define USB_DT_INTERFACE        0x04
#define USB_DT_ENDPOINT         0x05
#define USB_DT_DEVICE_QUALIFIER     0x06
#define USB_DT_OTHER_SPEED_CONFIG   0x07
#define USB_DT_INTERFACE_POWER      0x08
/* these are from a minor usb 2.0 revision (ECN) */
#define USB_DT_OTG             0x09
#define USB_DT_DEBUG           0x0a
#define USB_DT_INTERFACE_ASSOCIATION  0x0b

/* DSTS */
#define ENUM_SPEED_MASK    (0x3 << 1)
#define ENUM_SPEED_BIT     (1 << 1)
#define ENUM_SPEED_HIGH    (0x0 << 1)

/* DCFG */
#define DEV_ADDR_MASK  (0x7f << 4)
#define DEV_ADDR_BIT   4


#endif
