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

#include <usbdevice.h>

#include <usb/jz4780_udc.h>

#include <asm/arch/board_special.h>

#include "ep0.h"
#include "usb_cdc_acm.h"
#include "usbdescriptors.h"

/* defined same as usbtty.c */
#define ACM_TX_ENDPOINT 3
#define ACM_RX_ENDPOINT 2

#define TTY_PIPE_NAME "ttypipe"

#undef ttyinfo
#undef ttydbg
#undef ttyerr

#define ttyinfo(fmt,args...) serial_printf("ttyudc: " fmt, ##args)
#define ttyerr(fmt,args...) serial_printf("\033[1;31m" fmt "\033[0m", ##args);

#if 0
#define ttydbg(fmt,args...) serial_printf("tty: " fmt, ##args)
#define ttymsg(fmt,args...) serial_printf(fmt, ##args)
#else
#define ttydbg(fmt,args...) do{}while(0)
#define ttymsg(fmt,args...) do{}while(0)
#endif

volatile u32 g_flag = 0;

volatile static u32 f_overflow = 0;
#define set_data_overflow() do {f_overflow = 1;} while(0)
#define clean_data_overflow() do {f_overflow = 0;} while(0)
#define is_data_overflow() (f_overflow ? 1 : 0)

static struct usb_device_instance *tty_device;
static struct udc_pipe_dev *tty_pipe;
extern int extty_fill_buffer (void);

static struct usb_endpoint_instance *fepnum_instance (int num)
{
	u32 i;
	u32 ep_addr;
	u32 ep_num;
	struct usb_endpoint_instance *ep;

	for (i = 0; i < tty_device->bus->max_endpoints; i++) {
		ep = tty_device->bus->endpoint_array + i;
		ep_addr = ep->endpoint_address;
		ep_num = ep_addr & USB_ENDPOINT_NUMBER_MASK;

		if (ep_num == num)
			return &tty_device->bus->endpoint_array[i];
	}
	return NULL;
}

void udc_setup_ep(struct usb_device_instance *device,
	 unsigned int ep, struct usb_endpoint_instance *endpoint)
{
	ttydbg("%s\n", __func__);
}

int is_usbd_high_speed(void)
{
	ttydbg("%s %d\n", __func__, udc_high_speed());
	return udc_high_speed();
}

int udc_init(void)
{
	ttydbg("%s\n", __func__);
	return 0;
}

static void *tty_push_buffer(struct udc_pipe_dev *pipe_dev,
			unsigned int size)
{
	struct usb_endpoint_instance *ep;
	struct urb *urb;
	int ret;

	ep = fepnum_instance(ACM_TX_ENDPOINT);
	if(!ep) {
		ttyerr("cannot find ep%d instance\n", ACM_TX_ENDPOINT);
		return NULL;
	} else {
		if (!ep->rcv_urb)
			return NULL;
		urb = ep->rcv_urb;
	}

	usbd_rcv_complete (ep, size, 0);
	ret = extty_fill_buffer();
	if (!ret && size) {
		ttyerr ("tty circbuf Overflow %d %d\n", size, ret);
		set_data_overflow();
		return 0;
	}

	return (void *)urb->buffer;
}

/* Allow udc code to do any additional startup */
void udc_startup_events(struct usb_device_instance *device)
{
	int ret;
	struct urb *ep0_urb = usbd_alloc_urb(device,
				device->bus->endpoint_array);

	struct udc_pipe_dev *pipe_dev;

	ttydbg("%s\n", __func__);
	tty_device = device;

	if (!ep0_urb) {
		ttyerr("struct ep0 urb malloc error!\n");
		return;
	}

	ret = udc_func_register(ep0_urb);
	if(ret) {
		ttyerr("Cannot register usbtty function.\n");
		free(ep0_urb);
		return;
	}

	pipe_dev = malloc(sizeof(struct udc_pipe_dev));
	if (!pipe_dev) {
		ttyerr("struct pipe_dev malloc error!\n");
		return;
	}

	memset(pipe_dev, 0, sizeof(struct udc_pipe_dev));

	pipe_dev->name = TTY_PIPE_NAME;
	pipe_dev->f_dev_push_buffer = tty_push_buffer;

	ret = udc_pipe_register(pipe_dev);
	if(ret) {
		ttyerr("Cannot register %s data pipe.\n", TTY_PIPE_NAME);
		free(pipe_dev);
		return;
	}

	ret = pipe_ioctl_udc(pipe_dev, UDC_IOCTL_SET_BASE, 0);
	if(ret) {
		ttyerr("ioctl set %s data pipe base error.\n", TTY_PIPE_NAME);
		return;
	}

	tty_pipe = pipe_dev;

	/* The DEVICE_INIT event puts the USB device in the state STATE_INIT */
	usbd_device_event_irq(device, DEVICE_INIT, 0);

	/* The DEVICE_CREATE event puts the USB device in the state
	 * STATE_ATTACHED */
	usbd_device_event_irq(device, DEVICE_CREATE, 0);

	/* Some USB controller driver implementations signal
	 * DEVICE_HUB_CONFIGURED and DEVICE_RESET events here.
	 * DEVICE_HUB_CONFIGURED causes a transition to the state
	 * STATE_POWERED, and DEVICE_RESET causes a transition to
	 * the state STATE_DEFAULT.
	 */
}

/* Higher level functions for abstracting away from specific device */
int udc_endpoint_write(struct usb_endpoint_instance *endpoint)
{
	struct urb *urb = endpoint->tx_urb;
	ttydbg("%s %d\n", __func__, urb->actual_length);

	pipe_push_to_udc(tty_pipe, urb->buffer, urb->actual_length);

	urb->actual_length = 0;
	return 0;
}

void udc_disable(void)
{
	ttydbg("%s\n", __func__);
}

void udc_connect(void)
{
	ttydbg("%s\n", __func__);
}

void udc_disconnect(void)
{
	ttydbg("%s\n", __func__);
}

/* Flow control */
void udc_set_nak(int epid)
{
	/* TODO: implement this functionality in jz4780 */
}

void udc_unset_nak(int epid)
{
	/* TODO: implement this functionality in jz4780 */
}

void udc_irq(void)
{
	int ret = extty_fill_buffer();
	if (is_data_overflow() && ret) {
		pipe_ioctl_udc(tty_pipe, UDC_IOCTL_SET_BASE, 0);
		clean_data_overflow();
	}

	udc_process();
}

#if 0
const char text[] =
"The ARM comprehensive product offering includes 32-bit RISC microprocessors, "
"graphics processors, enabling software, cell libraries, embedded memories, "
"high-speed connectivity products, peripherals and development tools. Combined "
"with comprehensive design services, training, support and maintenance, "
"and the company's broad Partner community, they provide a total system solution "
"that offers a fast, reliable path to market for leading electronics companies.";

static int do_ttyxxx_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 ps, ts, i;
	void *p = memalign (CONFIG_SYS_CACHELINE_SIZE, 1024);

//	if (argc < 3)
//		return CMD_RET_USAGE;
//
//	ps = simple_strtoul(argv[1], NULL, 10);
//	ts = simple_strtoul(argv[2], NULL, 10);
//
//	if (ps > 1024)
//		ps = 1024;
//
//	memcpy(p, text, 1024);
//
//	for (i=0; i<ts; i++)
//		pipe_push_to_udc(tty_pipe, p, ps);
//
//	free (p);
//
	g_flag = !g_flag;
	if (!g_flag) {
		pipe_ioctl_udc(tty_pipe, UDC_IOCTL_SET_BASE, 0);
	}
	return 0;
}

U_BOOT_CMD(
	ttyxxx, 6, 1, do_ttyxxx_cmd,
	"tty function test command",
	"ttyxxx ps ts\n"
	);
#endif

