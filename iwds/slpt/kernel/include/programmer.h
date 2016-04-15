/*
 * Copyright (c) 2013 Ingenic Semiconductor, <lhhuang@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#define PRINT(x,...) printf("prog,"x"", ##__VA_ARGS__)
#define LOGD(x,...) printf("debug: %s(line %d): "x"", __func__, __LINE__, ##__VA_ARGS__)
#define LOGE(x,...) printf("error: %s(line %d): "x"", __func__, __LINE__, ##__VA_ARGS__)

#define PROG_ERROR "ERROR: "
#define PROG_WARN "WARN: "
#define PROG_INFO "INFO: "
#define PROG_READY "READY\n"

/* storage media type */
enum {
	TYPE_NAND,
	TYPE_MSC,
	TYPE_MEM
};

/* image type */
enum {
	TYPE_RAW,
	TYPE_UBI,
	TYPE_VNAND,
	TYPE_YAFFS,
	TYPE_JFFS2,
	TYPE_EXT4
};
