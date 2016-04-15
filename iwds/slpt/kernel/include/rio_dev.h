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

#ifndef _RIO_DEV_H_
#define _RIO_DEV_H_

#include <linux/ioctl.h>
#include <input.h>
#include <stdio_dev.h>

#define	RIO_IOCTL_BASE 'R'

#define RIO_IOCTL_SET_TIMEOUT _IOW(RIO_IOCTL_BASE, 1, short)
#define RIO_IOCTL_SET_PACKETC _IOW(RIO_IOCTL_BASE, 2, short)

struct rio_dev {
	char *name;
	void *priv;

	int (*open) (struct rio_dev *dev, int flags, mode_t mode);
	int (*close) (struct rio_dev *dev);
	ssize_t (*read) (struct rio_dev *dev, void *buf, size_t n);
	ssize_t (*write) (struct rio_dev *dev, const void *buf, size_t n);
	long (*ioctl) (struct rio_dev *dev, unsigned int cmd, unsigned long arg);
};

struct rio_dev *rio_open(const char *name, int flags, mode_t mode);
int rio_close(struct rio_dev *dev);
ssize_t rio_read(struct rio_dev *dev, void *buf, size_t n);
ssize_t rio_write(struct rio_dev *dev, const void *buf, size_t n);
long rio_ioctl(struct rio_dev *dev, unsigned int cmd, unsigned long arg);

#endif

