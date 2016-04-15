/*
 * Copyright (C) 2013 Ingenic Semiconductor Co., Ltd.
 * Authors: Kage Shen <kkshen@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 3 of the License, or (at your option) any later version.
 */

#ifndef __M200_UDC_H__
#define __M200_UDC_H__

#include <asm/byteorder.h>
#include <usbdevice.h>

/* Endpoint 0 states */
#define EP0_IDLE		0
#define EP0_IN_DATA		1
#define EP0_OUT_DATA		2
#define EP0_XFER_COMPLETE	3

/* Endpoint parameters */
#define DEP_PKTSIZE_MASK    0x7ff
#define DEP_FS_PKTSIZE      64
#define DEP_HS_PKTSIZE      512

#define MAX_ENDPOINTS       16
#define EP_MAX_PACKET_SIZE  64

#define UDC_BULK_HS_PACKET_SIZE DEP_HS_PKTSIZE
#define EP0_MAX_PACKET_SIZE     64
#define UDC_OUT_ENDPOINT        0x03
#define UDC_OUT_PACKET_SIZE     EP_MAX_PACKET_SIZE
#define UDC_IN_ENDPOINT         0x02
#define UDC_IN_PACKET_SIZE      EP_MAX_PACKET_SIZE
#define UDC_INT_ENDPOINT        0x01
#define UDC_INT_PACKET_SIZE     EP_MAX_PACKET_SIZE
#define UDC_BULK_PACKET_SIZE    EP_MAX_PACKET_SIZE

void udc_irq(void);
/* Flow control */
void udc_set_nak(int epid);
void udc_unset_nak(int epid);
void udc_clean_nak(int epid);

/* Higher level functions for abstracting away from specific device */
int udc_endpoint_write(struct usb_endpoint_instance *endpoint);

int  udc_init(void);

void udc_enable(struct usb_device_instance *device);
void udc_disable(void);

void udc_connect(void);
void udc_disconnect(void);

void udc_startup_events(struct usb_device_instance *device);
void udc_setup_ep(struct usb_device_instance *device,
	 unsigned int ep, struct usb_endpoint_instance *endpoint);

int udc_high_speed(void);

#if 0
u32 udc_entry_rio_mode(u32 epid, void *addr);
void udc_quit_rio_mode(u32 epid);

void udc_entry_quick_mode(void);
void udc_quit_quick_mode(void);
#endif

#include <linux/ioctl.h>

#define UDC_IOCTL_CODE 'U'
#define UDC_IOCTL_SET_BASE         _IOW(UDC_IOCTL_CODE, 1, short)
#define UDC_IOCTL_SET_CURRENT      _IOW(UDC_IOCTL_CODE, 2, short)
#define UDC_IOCTL_SET_BYPASS       _IOW(UDC_IOCTL_CODE, 3, short)
#define UDC_IOCTL_SET_AUTO_BYPASS  _IOW(UDC_IOCTL_CODE, 4, short)
#define UDC_IOCTL_SET_TIMEOUT      _IOW(UDC_IOCTL_CODE, 5, short)
#define UDC_IOCTL_BIG_PACKET       _IOW(UDC_IOCTL_CODE, 6, short)
#define UDC_IOCTL_NORMAL_PACKET    _IOW(UDC_IOCTL_CODE, 7, short)

struct udc_pipe_dev {
	char *name; /* Registers udv pipe name */
	/* udc push data to device buffer */
	void *(*f_dev_push_buffer)(struct udc_pipe_dev *pipe_dev,
			unsigned int size);
	/* devices push data to udc */
	int (*f_udc_push_buffer)(struct udc_pipe_dev *pipe_dev,
			void *b, unsigned int size);
	/* send io Control Command to udc device */
	int (*f_udc_ioctl) (struct udc_pipe_dev *pipe_dev,
			unsigned int cmd, unsigned long arg);
	/* Do not use the under variables */
	struct list_head list;
	unsigned int bypass_length;
	unsigned int flag;
	void *priv;
};

int pipe_push_to_udc(struct udc_pipe_dev *pipe_dev,
		void *b, unsigned int size);
int pipe_ioctl_udc(struct udc_pipe_dev *pipe_dev,
		unsigned int cmd, unsigned long arg);
void *pipe_push_to_dev(struct udc_pipe_dev *pipe_dev,
		unsigned int size);

void udc_process(void);

int udc_func_register(struct urb *ep0_urb);
int udc_pipe_register(struct udc_pipe_dev *pipe_dev);

#endif

