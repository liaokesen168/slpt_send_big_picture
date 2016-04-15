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

#include <asm/unaligned.h>

#include <linux/err.h>
#include <asm/io.h>
#include <rio_dev.h>

#define rioinfo(fmt,args...) serial_printf(fmt, ##args)
#define rioerr(fmt,args...) serial_printf("\033[1;31m" fmt "\033[0m", ##args);

#if 0
#define riodbg(fmt,args...) serial_printf("rio: " fmt, ##args)
#define riomsg(fmt,args...) serial_printf(fmt, ##args)
#else
#define riodbg(fmt,args...) do{}while(0)
#define riomsg(fmt,args...) do{}while(0)
#endif

struct rio_dev *rio_open(const char *name, int flags, mode_t mode)
{
	struct rio_dev *rdev;

	struct stdio_dev *sdev = stdio_get_by_name(name);
	if(sdev == NULL) {
		rioinfo("Cannot open stdio device %s\n", name);
		return ERR_PTR(-ENODEV);
	}

	/* Rio (Robust I/O), stdio_dev.priv->rio_dev */
	if(sdev->priv == NULL) {
		rioinfo("stdio dev %s haven't register rio dev\n", name);
		return ERR_PTR(-EBADR);
	}

	rdev = (struct rio_dev *)sdev->priv;
	rdev->open(rdev, flags, mode);

	return rdev;
}

int rio_close(struct rio_dev *dev)
{
	return dev->close(dev);
}

ssize_t rio_read(struct rio_dev *dev, void *buf, size_t n)
{
	return dev->read(dev, buf, n);
}

ssize_t rio_write(struct rio_dev *dev, const void *buf, size_t n)
{
	return dev->write(dev, buf, n);
}

long rio_ioctl(struct rio_dev *dev, unsigned int cmd, unsigned long arg)
{
	return dev->ioctl(dev, cmd, arg);
}

