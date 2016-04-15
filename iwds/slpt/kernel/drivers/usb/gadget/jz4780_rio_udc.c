/*
 * Copyright (C) 2013 Ingenic Semiconductor Co., Ltd.
 * Authors: Kage Shen <kkshen@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 3 of the License, or (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include <config.h>
#include <common.h>
#include <malloc.h>

#include <circbuf.h>
#include <stdio_dev.h>
#include <asm/unaligned.h>

#include <linux/err.h>
#include <asm/io.h>

#include <rio_dev.h>

#include <usb/jz4780_udc.h>

#include <hw_sha.h>
#include <hash.h>
#include <sha1.h>
#include <sha256.h>

#define rioinfo(fmt,args...) serial_printf("rio: " fmt, ##args)
#define rioerr(fmt,args...) serial_printf("\033[1;31m" fmt "\033[0m", ##args);

#if 0
#define riodbg(fmt, args...) serial_printf("rio: " fmt, ##args)
#define riomsg(fmt, args...) serial_printf(fmt, ##args)
#else
#define riodbg(fmt,args...) do{}while(0)
#define riomsg(fmt,args...) do{}while(0)
#endif

volatile static u32 f_overflow = 0;
#define set_data_overflow() do {f_overflow = 1;} while(0)
#define clean_data_overflow() do {f_overflow = 0;} while(0)
#define is_data_overflow() (f_overflow ? 1 : 0)

/*
 * part 1:
 *                   +----------------------+
 *                   | rio_circbufs_manager |   --- manager
 *                   +-----+----------------+
 *  ~> cdata       ~> ctop |  ~> ctail            ~> cend
 *  +---------------------------------------------+
 *  | circbuf[0] | circbuf[1] | ... |  * nchild   |
 *  +---------------------------------------------+--------+
 *  |               udc_buf (psize * nchild)               |
 *  +------------------------------------------------------+
 *
 * part 2:
 *  +------------+
 *  |  *circbuf  |
 *  +------------+
 *         |
 *  +----------------------------+
 *  |  1 * psize                 |
 *  +----------------------------+
 *  ^     ^             ^        ^
 *  data  top           tail     end
 */

struct rio_circbufs_manager {
	u32 buf_size; /* total buffer size */
	void *buf;

	circbuf_t *ctop; /* pointer to current buffer start */
	circbuf_t *ctail; /* pointer to space for next element */

	circbuf_t *cdata; /* all circbufs unit data start addr */
	circbuf_t *cend; /* end of child circbufs unit range */
};

struct rio_params_manager {
	struct rio_dev *rdev;
	struct udc_pipe_dev *rio_pipe;
	struct rio_circbufs_manager *rio_circbufs;
	u32 packet_count;
	u32 packet_size;
	u32 read_timeout;
	u32 read_buffer_full;
};

struct rio_params_manager *rio_params;

/* defined same as usbtty.c */
#define ACM_TX_ENDPOINT 3
#define ACM_RX_ENDPOINT 2

#define RIO_PIPE_NAME "riopipe"
#define RIO_UDC_NAME "rioudc"

static void rio_dev_register(void)
{
	struct stdio_dev *sdev;
	char *name = "usbtty";
	sdev = stdio_get_by_name(name);
	if(sdev == NULL) {
		rioerr("Cannot open stdio device %s\n", name);
		return;
	}

	sdev->priv = rio_params->rdev;
}

static void *rio_push_buffer(struct udc_pipe_dev *pipe_dev,
			unsigned int count)
{
	struct rio_circbufs_manager *prio_circbufs;
	circbuf_t *circbuf;

	prio_circbufs = rio_params->rio_circbufs;
	/* first update current circbuf status */
	circbuf = prio_circbufs->ctail;
	/* update current circbuf */
	circbuf->top = circbuf->data;
	circbuf->size = count;

	riodbg("fill->[%p].data [%p] count [%d] %c\n",
			circbuf, circbuf->data, count, circbuf->data[0]);

	if (!count)
		return (void *)circbuf->data;

	riodbg ("crc32 %p %d -> %x\n", circbuf->data, count,
			crc32_wd(0, (const uchar *)circbuf->data, count, CHUNKSZ_CRC32));

	/* pointer a new struct circbuf */
	circbuf = circbuf + 1;
	if((u32)circbuf == (u32)prio_circbufs->cend) {
		circbuf = prio_circbufs->cdata;
	}

	prio_circbufs->ctail = circbuf;
	if((u32)circbuf == (u32)prio_circbufs->ctop) {
		rioerr("Rio buffer is full (T.T) ...\n");
		set_data_overflow();
		return NULL;
	}

	return (void *)circbuf->data;
}

static int rio_circbuf_pop(circbuf_t *circbuf, void *buf, u32 size)
{
	u32 count = MIN(size, circbuf->size);

	memcpy(buf, circbuf->top, count);

	riodbg("pop->[%p].data [%p] count [%d]\n",
			circbuf, circbuf->top, count);
	circbuf->size -= count;
	circbuf->top += count;

	return count;
}

/* This function is no block, if buffer is empty
 * that function return 0 */
static u32 take_rio_buffer(void *buf, u32 size)
{
	struct rio_circbufs_manager *prio_circbufs;
	circbuf_t *circbuf;

	u32 count = 0;
	u32 tmp = 0;

	if(!size)
		return 0;

	prio_circbufs = rio_params->rio_circbufs;
	circbuf = prio_circbufs->ctop;
	/* if current circbuf is empty return 0 */
	if(circbuf->size == 0)
		return 0;

	do {
		tmp = rio_circbuf_pop (circbuf, buf + count, size - count);
		if(circbuf->size == 0) {
			prio_circbufs->ctop ++;
			if(prio_circbufs->ctop == prio_circbufs->cend)
				prio_circbufs->ctop = prio_circbufs->cdata;

			circbuf = prio_circbufs->ctop;
			/* this time udc_buf is empty */
			if(circbuf->size == 0)
				return (count + tmp);
		}

		count += tmp;
	} while(count < size);

	return count;
}

static void reset_rio_circbufs (struct rio_params_manager *prio_params)
{
	struct rio_circbufs_manager *prio_circbufs;
	u32 count = prio_params->packet_count;
	u32 psize = prio_params->packet_size;
	u32 i;

	prio_circbufs = rio_params->rio_circbufs;

	for (i = 0; i < count; i++) {
		circbuf_t *p = prio_circbufs->cdata + i;
		p->size = 0;
		p->data = ((char *)prio_circbufs->buf + (i * psize));
		p->top = p->data;
	}

	prio_circbufs->ctop = prio_circbufs->cdata;
	prio_circbufs->ctail = prio_circbufs->cdata;
	prio_circbufs->cend = (prio_circbufs->cdata + count);
}

static int rio_udc_open (struct rio_dev *dev, int flags, mode_t mode)
{
	struct rio_params_manager *prio_params = dev->priv;

	reset_rio_circbufs(prio_params);

	pipe_ioctl_udc(prio_params->rio_pipe, UDC_IOCTL_SET_CURRENT, 0);

	return 0;
}

static int rio_udc_close (struct rio_dev *dev)
{
	pipe_ioctl_udc(rio_params->rio_pipe, UDC_IOCTL_SET_BYPASS, 0);
	return 0;
}

static ssize_t rio_udc_write (struct rio_dev *dev, const void *buf, size_t n)
{
	int ret;

	if (!n)
		return 0;

	ret = pipe_push_to_udc(rio_params->rio_pipe, (void *)buf, n);
	if (ret < 0) {
		rioerr ("rio write error, erron %d\n", ret);
	}

	return ret;
}

/* This function is block, if buffer is empty ro data transfer
 * completed that return read byte size */
static ssize_t rio_udc_read (struct rio_dev *dev, void *buf, size_t n)
{
	u32 count = 0;
	u32 time_out = get_timer(0);

	if (n == 0)
		return 0;

	if (n > 0x200000)
		pipe_ioctl_udc(rio_params->rio_pipe,
				UDC_IOCTL_BIG_PACKET, rio_params->packet_size);
	else
		pipe_ioctl_udc(rio_params->rio_pipe, UDC_IOCTL_NORMAL_PACKET, 0);

	udc_process();

	while(count < n) {
		u32 i = take_rio_buffer((u8 *)buf + count, n - count);
		if (i) {
			time_out = get_timer(0);
			count += i;
		} else if (get_timer(time_out) > rio_params->read_timeout) {
			rioerr("rio read time out, has received %d bytes!\n", count);
			return -ETIME;
		} else if (is_data_overflow()) {
			pipe_ioctl_udc(rio_params->rio_pipe, UDC_IOCTL_SET_BASE, 0);
			clean_data_overflow();
		}

		if ((n - count) < 0x100000)
			pipe_ioctl_udc(rio_params->rio_pipe, UDC_IOCTL_NORMAL_PACKET, 0);

		udc_process();
	}

	return count;
}

static long rio_udc_ioctl(struct rio_dev *dev, unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
	case RIO_IOCTL_SET_TIMEOUT:
		if (arg)
			rio_params->read_timeout = arg;
		break;
	case RIO_IOCTL_SET_PACKETC:
		break;
	default:
		rioinfo("ioctl unknown command %d\n", cmd);
		break;
	}
	return 0;
}

int udc_rio_startup(void)
{
	struct rio_dev *rdev;
	struct udc_pipe_dev *pipe_dev;
	struct rio_circbufs_manager *prio_circbufs;
	struct rio_params_manager *prio_params;
	int ret;

	rioerr("%s\n", __func__);

	rdev = malloc(sizeof(struct rio_dev));
	if (!rdev) {
		rioerr("struct rdev malloc error!\n");
		return -1;
	}

	prio_circbufs = malloc(sizeof(struct rio_circbufs_manager));
	if (!prio_circbufs) {
		rioerr("struct rio_circbufs_manages malloc error!\n");
		goto ERROR_RIO_CIRC;
	}

	pipe_dev = malloc(sizeof(struct udc_pipe_dev));
	if (!pipe_dev) {
		rioerr("struct pipe_dev malloc error!\n");
		goto ERROR_RIO_PIPE;
	}

	prio_params = malloc(sizeof(struct rio_params_manager));
	if (!prio_params) {
		rioerr("struct rio_params_manager malloc error!\n");
		goto ERROR_RIO_PARAMS;
	}

	memset(rdev, 0, sizeof(struct rio_dev));
	memset(prio_circbufs, 0, sizeof(struct rio_circbufs_manager));
	memset(pipe_dev, 0, sizeof(struct udc_pipe_dev));
	memset(prio_params, 0, sizeof(struct rio_params_manager));

	pipe_dev->name = RIO_PIPE_NAME;
	pipe_dev->f_dev_push_buffer = rio_push_buffer;

	ret = udc_pipe_register(pipe_dev);
	if(ret) {
		rioerr("Cannot register %s data pipe.\n", RIO_PIPE_NAME);
		goto ERROR_RIO_PARAMS;
	}

	rdev->priv = prio_params;
	rdev->name = RIO_UDC_NAME;

	rdev->open = rio_udc_open;
	rdev->close = rio_udc_close;
	rdev->read = rio_udc_read;
	rdev->write = rio_udc_write;
	rdev->ioctl = rio_udc_ioctl;

	prio_params->rdev = rdev;
	prio_params->rio_circbufs = prio_circbufs;
	prio_params->rio_pipe = pipe_dev;
	prio_params->read_timeout = 3000;
	prio_params->packet_size = 1024 * 3;
	prio_params->packet_count = 8;

	rio_params = prio_params;

	prio_circbufs->buf_size =
		prio_params->packet_size * prio_params->packet_count;
	prio_circbufs->buf = (void *) memalign(CONFIG_SYS_CACHELINE_SIZE,
			prio_circbufs->buf_size);

	if (!prio_circbufs->buf) {
		rioerr("unable to allocate udc buffer %d bytes\n",
				prio_circbufs->buf_size);
		goto ERROR_RIO_PARAMS;
	}

	memset(prio_circbufs->buf, 0, prio_circbufs->buf_size);

	prio_circbufs->cdata = (circbuf_t *) memalign(CONFIG_SYS_CACHELINE_SIZE,
			sizeof(circbuf_t) * prio_params->packet_count);

	if (!prio_circbufs->cdata) {
		rioerr("unable to allocate udc circbuf * %d\n",
				prio_params->packet_count);
		goto ERROR_RIO_BCIRC;
	}
	memset(prio_circbufs->cdata, 0,
			sizeof(circbuf_t) * prio_params->packet_count);

	reset_rio_circbufs (rio_params);

	rio_dev_register();

	return 0;

ERROR_RIO_BCIRC:
	free (prio_circbufs->buf);
ERROR_RIO_PARAMS:
	free (pipe_dev);
ERROR_RIO_PIPE:
	free (prio_circbufs);
ERROR_RIO_CIRC:
	free (rdev);
	return -1;
}

#if 0
static int do_rioxxx_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 cmd;

	if (argc < 2)
		return CMD_RET_USAGE;

	cmd = simple_strtoul(argv[1], NULL, 10);

	switch (cmd) {
	case 1:
		pipe_ioctl_udc(rio_params->rio_pipe, UDC_IOCTL_SET_CURRENT, 0);
		break;
	case 2:
		pipe_ioctl_udc(rio_params->rio_pipe, UDC_IOCTL_SET_BYPASS, 0);
		break;
	case 3:
		break;
	case 4:
		break;
	}

	return 0;
}

U_BOOT_CMD(
	rioxxx, 6, 1, do_rioxxx_cmd,
	"rio function test command",
	"rioxxx xxxxxx\n"
	);
#endif

