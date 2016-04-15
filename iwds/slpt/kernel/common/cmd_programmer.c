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
 /*
 * TODO:
 * 1. If need host check, check on host;
 * 2. mmc support;
 * 3. support read command.
 */

#include <common.h>
#include <command.h>
#include <nand.h>

#include <malloc.h>
#include <stdio_dev.h>
#include <rio_dev.h>
#include <div64.h>
#include <mmc.h>

#include <jffs2/load_kernel.h>
#include <linux/list.h>
#include <linux/ctype.h>
#include <linux/err.h>
#include <linux/mtd/mtd.h>

#include <jffs2/jffs2.h>
#if defined(CONFIG_CMD_NAND)
#include <linux/mtd/nand.h>
#include <nand.h>
#endif

#if defined(CONFIG_CMD_ONENAND)
#include <linux/mtd/onenand.h>
#include <onenand_uboot.h>
#endif

#include <asm/arch/board_special.h>
#include <programmer.h>

typedef struct erase_info	erase_info_t;
typedef struct mtd_info		mtd_info_t;

struct spl_params {
	uint8_t busw;
	uint8_t toggle;
	uint8_t rowcycle;
	uint8_t bak_blocks;
	uint32_t pagesize;
	uint32_t size;
};

/* support only for native endian JFFS2 */
#define cpu_to_je16(x) (x)
#define cpu_to_je32(x) (x)

#define CONFIG_CMD_NAND_TRIMFFS
#define CONFIG_CMD_NAND_YAFFS

/*
 * parameters for caching images
 * @blksz: the actual memory size for caching images
 * @min: the minimal memory size for caching images
 * @step: the step memory size for caching images
 * @max: the maximum memory size for caching images
 *
 * cache_params[1] for caching yaffs for NAND;
 * cache_params[0] for caching other image type.

 **/
struct cache_param {
	loff_t blksz;
	loff_t min;
	loff_t step;
	loff_t max;
} cache_params[2] = {{0}};

/* whether the programmer is running on ingenic platform */
static bool is_ingenic = true;

/* partition handling routines */
extern int mtdparts_init(void);
extern int find_dev_and_part(const char *id, struct mtd_device **dev, u8 *part_num,
		struct part_info **part);

extern char *strerror(int num);

static char *prog_strerror(int num)
{
	return strerror(num);
}

/**
 * nand_check_wp - [GENERIC] check if the chip is write protected
 * @mtd: MTD device structure
 *
 * Check, if the device is write protected. The function expects, that the
 * device is already selected.
 */
static int nand_check_wp(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;

	/* Broken xD cards report WP despite being writable */
	if (chip->options & NAND_BROKEN_XD)
		return 0;

	/* Check the WP bit */
	chip->cmdfunc(mtd, NAND_CMD_STATUS, -1, -1);
	return (chip->read_byte(mtd) & NAND_STATUS_WP) ? 0 : 1;
}

static inline int str2off(const char *p, loff_t *num)
{
	char *endptr;

	*num = simple_strtoull(p, &endptr, 16);
	return *p != '\0' && *endptr == '\0';
}

static inline int str2long(const char *p, ulong *num)
{
	char *endptr;

	*num = simple_strtoul(p, &endptr, 16);
	return *p != '\0' && *endptr == '\0';
}

static int set_dev(int dev)
{
	if (dev < 0 || dev >= CONFIG_SYS_MAX_NAND_DEVICE ||
			!nand_info[dev].name) {
		puts("No such device\n");
		return -1;
	}

	if (nand_curr_device == dev)
		return 0;

	LOGD("Device %d: %s", dev, nand_info[dev].name);
	LOGD("... is now current device\n");
	nand_curr_device = dev;

	return 0;
}

static int get_part(const char *partname, int *idx, loff_t *off,
		loff_t *size, loff_t *maxsize)
{
#ifdef CONFIG_CMD_MTDPARTS
	struct mtd_device *dev;
	struct part_info *part;
	u8 pnum;
	int ret;

	ret = mtdparts_init();
	if (ret)
	return ret;

	ret = find_dev_and_part(partname, &dev, &pnum, &part);
	if (ret)
	return ret;

	if (dev->id->type != MTD_DEV_TYPE_NAND) {
		PRINT(PROG_ERROR "not a NAND device\n");
		return -1;
	}

	*off = part->offset;
	*size = part->size;
	*maxsize = part->size;
	*idx = dev->id->num;

	ret = set_dev(*idx);
	if (ret)
	return ret;

	return 0;
#else
	puts("offset is not a number\n");
	return -1;
#endif
}

static int arg_off(const char *arg,int *idx, loff_t *off,
		loff_t *size, loff_t *maxsize, bool *is_part)
{
	int ret;

	if (is_part)
		*is_part = false;

	if (!strcmp(arg, "chip")) {
		*off = 0;
		*size = nand_info[*idx].size;
		*maxsize = *size;
		if (is_part)
			*is_part = true;
		return 0;
	}

	if (!str2off(arg, off)) {
		ret = get_part(arg, idx, off, size, maxsize);
		LOGD("ret = %d addr of is_part %p\n", ret, is_part);
		if (ret != 0)
			PRINT(PROG_ERROR "%s isn't a valid partition name!\n", arg);
		else if (is_part)
			*is_part = true;

		return ret;
	}

	if (*off >= nand_info[*idx].size) {
		PRINT(PROG_ERROR "Offset exceeds device limit\n");
		return -1;
	}

	*maxsize = nand_info[*idx].size - *off;
	*size = *maxsize;
	return 0;
}

static int arg_off_size(int argc, char * const argv[], int *idx,
		loff_t *off, loff_t *size, loff_t *maxsize, bool *is_part)
{
	int ret;

	ret = arg_off(argv[0], idx, off, size, maxsize, is_part);
	if (ret)
		return ret;

	if (argc == 1)
		goto print;

	if (!str2off(argv[1], size)) {
		PRINT(PROG_ERROR "'%s' is not a number\n", argv[1]);
		return -1;
	}

	if (*size > *maxsize) {
		PRINT(PROG_ERROR "Size exceeds partition or device limit\n");
		return -1;
	}

print:
	LOGD("device %d ", *idx);
	if (*size == nand_info[*idx].size)
		LOGD("whole chip\n");
	else
		LOGD("offset 0x%llx, size 0x%llx\n", (unsigned long long) *off,
				(unsigned long long) *size);
	return 0;
}

/**
 *  Query all storage media on the device.
 */
static void query_device(void)
{
	int i;
	struct mmc *m;
	PRINT(PROG_INFO "name,type,size\n");
	for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++) {
		if (nand_info[i].name) {
			struct nand_chip *chip = (struct nand_chip *)nand_info[i].priv;
			PRINT(PROG_INFO "%s,NAND,%#llx\n",
					nand_info[i].name, (unsigned long long)chip->chipsize);
		}
	}

	LOGD("mmc_num %d\n", get_mmc_num());
	for (i = 0; i < get_mmc_num(); i++) {
		if ((m = find_mmc_device(i)) != NULL) {
			if (mmc_init(m) == 0) {
				PRINT(PROG_INFO "%s,%s,%#llx\n", m->name,
						IS_SD(m) ? "SD" : "MMC", m->capacity);
			} else
				LOGE("mmc init failed!\n");
		}
	}
	PRINT(PROG_INFO "mem,MEM,%#x\n", misc_param.mem_size);
}

/**
 * Query all partitions in the specified nand device.
 *
 * @param name - pointer to the nand device name string
 * @return 0 on success, 0 otherwise
 */
static int query_nand_part(struct mtd_info *mtd)
{
#ifdef CONFIG_CMD_MTDPARTS
	struct mtd_device *dev;
	struct part_info *part;
	u8 pnum;
	int nand_idx, j = 0, ret, num_parts, writable;
	char partname[10];
	const char *nandname;

	ret = mtdparts_init();
	if (ret)
		return ret;

	nand_idx = mtd - nand_info;
	nandname = nand_info[nand_idx].name;

	if (nandname == NULL || strncmp(nandname, "nand", 4) != 0) {
		PRINT(PROG_ERROR "No such nand device!\n");
		return -1;
	}
	dev = device_find(MTD_DEV_TYPE_NAND, nand_idx);
	if (dev != NULL)
		num_parts = dev->num_parts;
	else
		return -1;

	LOGD("num of parts = %d\n", num_parts);
	writable = !nand_check_wp(mtd);
	PRINT(PROG_INFO "name,type,size,offset,writable\n");

	for (j = 0; j < num_parts; j++) {
		sprintf(partname, "nand%d,%d", nand_idx, j);
		ret = find_dev_and_part(partname, &dev, &pnum, &part);
		if (ret)
			break;

		// TODO: type
		PRINT(PROG_INFO "%s,%#x,%#llx,%#llx,%d\n", part->name, 0,
				part->size, part->offset, writable);
	}
#endif

	return 0;
}

static int query_nand_spl_params(struct mtd_info *mtd,
		struct spl_params *spl)
{
	uint8_t pageshift;
	struct otp_info spl_info;

	spl->bak_blocks = mtd->_get_user_prot_info(mtd, &spl_info,
			sizeof(spl_info)) / sizeof(struct otp_info);

	spl->busw = (uint8_t)(spl_info.locked >> 24);
	spl->toggle = (uint8_t)(spl_info.locked >> 16);
	spl->rowcycle = (uint8_t)(spl_info.locked >> 8);
	pageshift = (uint8_t)spl_info.locked;
	spl->pagesize = 1 << pageshift;
	spl->size = spl_info.length;


	LOGD("buswidth=%d, toggle=%d, rowcycle=%d, pagesize=%d, "
			"splsize=%d, number of spl backup copies = %d\n",
			(uint32_t)spl->busw, (uint32_t)spl->toggle,
			(uint32_t)spl->rowcycle, spl->pagesize, spl->size,
			(uint32_t)spl->bak_blocks);

	PRINT(PROG_INFO "%d,%d,%d,%d,%d,%d\n", (uint32_t)spl->busw,
			(uint32_t)spl->toggle, (uint32_t)spl->rowcycle,
			spl->pagesize, spl->size, (uint32_t)spl->bak_blocks);

	return 0;
}

static int query_nand_spl_erasecount(struct mtd_info *mtd,
		uint32_t *erasecount)
{
	int ret = 0;
	uint32_t cnt;

	if (is_ingenic) {
		loff_t off = mtd->erasesize - mtd->writesize;
		size_t len = 8;
		uint32_t buf[2];
		uint8_t pattern[4] = {'A', 'K', '4', '7'};
		uint8_t *ptr = (uint8_t *)buf;
		uint32_t *p_pattern = (uint32_t *)pattern;

		ret = nand_read(mtd, off, &len, (u_char *)buf);

		LOGD("buf[0] %c %c %c %c buf[1] %#x \n", ptr[0], ptr[1], ptr[2], ptr[3], buf[1]);

		if (ret != 0) {
			PRINT(PROG_WARN "Getting erase count from %#llx failed:"
					"%s\n", off, prog_strerror(ret));
			return ret;
		} else if (buf[0] == 0xffffffff) {
			cnt = 0;
			LOGD("It's a new flash!\n");
		} else if (buf[0] != *p_pattern) {
			PRINT(PROG_WARN "Getting erase count failed: tag isn't "
					"AK47.\n");
			return -1;
		} else
			cnt = buf[1];

		if (erasecount)
			*erasecount = cnt;

		LOGD("Query SPL erase count(=%d).\n", cnt);
		PRINT(PROG_INFO "%d\n", cnt);
	} else {
		PRINT(PROG_WARN "Query erase count isn't support!\n");
		ret = -1;
	}

	return ret;
}

/**
 * Set the memory size for caching images in device.
 * @param image_type - image type, e.g TYPE_UBI, TYPE_YAFFS, TYPE_RAW
 * @param size - size of caching images to be set
 *
 * @return 0 on success, otherwise 1
 */
static int set_blksz(int image_type, loff_t size)
{
	char *buf = NULL;
	int index;

	if (image_type < 0) {
		PRINT(PROG_ERROR "image type error!\n");
		return 1;
	}

	index = (image_type == TYPE_YAFFS)? 1 : 0;

	if (size < cache_params[index].min) {
		PRINT(PROG_ERROR "smaller than min(%#llx)!\n",
				(unsigned long long)cache_params[index].min);
		return 1;
	}

	if (size > cache_params[index].max) {
		PRINT(PROG_ERROR "bigger than max(%#llx)!\n",
				(unsigned long long)cache_params[index].max);
		return 1;
	}

	if ((ulong) size % (ulong) cache_params[index].step != 0) {
		PRINT(PROG_ERROR "not aligned with step(%#llx)!\n",
				(unsigned long long)cache_params[index].step);
		return 1;
	}

	/* just try to request the required memory, if ok, free it immediately */
	buf = memalign(CONFIG_SYS_CACHELINE_SIZE, size);
	if (buf == NULL) {
		PRINT(PROG_ERROR "could not allocate enough memory,"
				" please set smaller blksz!\n");
		return 1;
	} else {
		free(buf);
		buf = NULL;
		cache_params[index].blksz = size;
		PRINT(PROG_INFO "%#llx\n",
				(unsigned long long)cache_params[index].blksz);
	}

	return 0;
}

/**
 * Scan bad block using standard method and re-mark bad block using
 * the method of ingenic vnand management layer.
 * @mtd - MTD device structure
 * @off - offset relative to nand start
 * @size - size to be scaned and re-marked
 * @ rescan - whether force to rescan
 *
 * @return 0 on success, otherwise non-zero
 */
static int vnand_scan_badblock(struct mtd_info *mtd, loff_t off,
		loff_t size, bool rescan)
{
	static uint8_t *bbt_bak = NULL;
	struct nand_chip *chip = mtd->priv;
	loff_t ofs;
	int ret = 0;

	if (bbt_bak == NULL || rescan) {
		if ((bbt_bak != NULL) && rescan) {
			free(bbt_bak);
			bbt_bak = NULL;
		}
		LOGD("Scan bad block for VNAND.\n");
		ret = chip->scan_bbt(mtd);
		if (ret)
			PRINT(PROG_ERROR "can't scan NAND and build the RAM-based"
					"for VNAND.\n");
		/* invalidate page cache, because it was used during scanning bbt*/
		chip->pagebuf = -1;
	} else {
		chip->bbt = bbt_bak;
	}

	for (ofs = off; ofs < off + size; ofs += mtd->erasesize) {
		int bad0, bad1;
		bad0 = mtd->_block_isbad(mtd, ofs); /* standard method */
		bad1= chip->block_bad(mtd, ofs, 1); /* vnand method */

		/*LOGD("scan at 0x%llx, bad0=%d bad1=%d\n", ofs, bad0, bad1);*/
		if (bad0 != bad1) {
			LOGD("Marking bad at 0x%llx, bad0=%d bad1=%d\n",
					ofs, bad0, bad1);
			ret = chip->block_markbad(mtd, ofs);
			if (ret)
				PRINT(PROG_WARN "Marking bad failed at 0x%llx\n", ofs);
		}
	}

	if (bbt_bak == NULL)
		bbt_bak = chip->bbt;
	chip->bbt = NULL;

	return ret;
}

static void mark_nand_spl_erasecount(struct mtd_info *mtd,
		uint32_t erasecount)
{
	int ret;

	if (is_ingenic) {
		loff_t off = mtd->erasesize - mtd->writesize;
		size_t len = 8;
		uint32_t buf[2];
		uint8_t pattern[4] = {'A', 'K', '4', '7'};
		uint32_t *ptr = (uint32_t *)pattern;

		LOGD("Marking SPL erase count %d at %#llx.\n", erasecount,
				off);

		buf[0] = *ptr;
		buf[1] = erasecount;
		ret = nand_write(mtd, off, &len, (u_char *)buf);
		if (ret != 0)
			PRINT(PROG_WARN "Marking erase count at %#llx failed: %s\n",
					off, prog_strerror(ret));
	}
}
/**
 * prog_nand_erase_opts: - erase NAND flash with support for various options
 *
 * @param meminfo	NAND device to erase
 * @param opts		options,  @see struct nand_erase_options
 * @return		0 in case of success
 *
 * This code is ported from flash_eraseall.c from Linux mtd utils by
 * Arcom Control System Ltd.
 */
static int prog_nand_erase_opts(nand_info_t *meminfo, const nand_erase_options_t *opts)
{
	struct jffs2_unknown_node cleanmarker;
	erase_info_t erase;
	unsigned long erase_length, erased_length; /* in blocks */
	int bbtest = 1;
	int result;
	int percent_complete = -1;
	struct mtd_oob_ops oob_opts;
	struct nand_chip *chip = meminfo->priv;
	uint32_t erasecount, ret = 0; /* for marking SPL erase count */

	if ((opts->offset & (meminfo->erasesize - 1)) != 0) {
		PRINT(PROG_ERROR "Attempt to erase non block-aligned data\n");
		return -1;
	}

	memset(&erase, 0, sizeof(erase));
	memset(&oob_opts, 0, sizeof(oob_opts));

	erase.mtd = meminfo;
	erase.len  = meminfo->erasesize;
	erase.addr = opts->offset;
	erase_length = lldiv(opts->length + meminfo->erasesize - 1,
			     meminfo->erasesize);

	cleanmarker.magic = cpu_to_je16(JFFS2_MAGIC_BITMASK);
	cleanmarker.nodetype = cpu_to_je16(JFFS2_NODETYPE_CLEANMARKER);
	cleanmarker.totlen = cpu_to_je32(8);

	/* scrub option allows to erase badblock. To prevent internal
	 * check from erase() method, set block check method to dummy
	 * and disable bad block table while erasing.
	 */
	if (opts->scrub) {
		erase.scrub = opts->scrub;
		/*
		 * We don't need the bad block table anymore...
		 * after scrub, there are no bad blocks left!
		 */
		if (chip->bbt) {
			kfree(chip->bbt);
		}
		chip->bbt = NULL;
	}

	for (erased_length = 0;
	     erased_length < erase_length;
	     erase.addr += meminfo->erasesize) {

		if (opts->lim && (erase.addr >= (opts->offset + opts->lim))) {
			PRINT(PROG_ERROR "Size of erase exceeds limit\n");
			return -EFBIG;
		}
		if (!opts->scrub && bbtest) {
			int ret = mtd_block_isbad(meminfo, erase.addr);
			if (ret > 0) {
				if (!opts->quiet)
					PRINT(PROG_INFO "Skipping bad block at 0x%08llx\n",
							erase.addr);
				if (!opts->spread)
					erased_length++;

				continue;

			} else if (ret < 0) {
				PRINT(PROG_ERROR "Get bad block failed!\n");
				return -1;
			}
		}

		erased_length++;

		if (erase.addr == 0)
			ret = query_nand_spl_erasecount(meminfo, &erasecount);

		result = mtd_erase(meminfo, &erase);
		if (result != 0) {
			PRINT(PROG_ERROR "NAND Erase failure: %s\n",
					prog_strerror(result));
#if 1
			if (opts->spread)
				erased_length--;
			PRINT(PROG_INFO "Mark bad at %#llx\n", erase.addr);
			meminfo->_block_markbad(meminfo, erase.addr);
#endif
			continue;
		}

		/* if querying SPL erase count succeeds, increase and mark */
		if (ret == 0 && erase.addr == 0)
			mark_nand_spl_erasecount(meminfo, erasecount + 1);

		/* format for JFFS2 ? */
		if (opts->jffs2 && chip->ecc.layout->oobavail >= 8) {
			struct mtd_oob_ops ops;
			ops.ooblen = 8;
			ops.datbuf = NULL;
			ops.oobbuf = (uint8_t *)&cleanmarker;
			ops.ooboffs = 0;
			ops.mode = MTD_OPS_AUTO_OOB;

			result = mtd_write_oob(meminfo,
			                            erase.addr,
			                            &ops);
			if (result != 0) {
				PRINT(PROG_ERROR "NAND writeoob failure: %s\n",
						prog_strerror(result));
				continue;
			}
		}

		if (!opts->quiet) {
			unsigned long long n = erased_length * 100ULL;
			int percent;

			do_div(n, erase_length);
			percent = (int)n;

			/* output progress message only at whole percent
			 * steps to reduce the number of messages printed
			 * on (slow) serial consoles
			 */
			if (percent != percent_complete) {
				percent_complete = percent;

				PRINT(PROG_INFO "progress %d, erasing at 0x%llx\n",
						percent, erase.addr);

				if (opts->jffs2 && result == 0)
					PRINT(PROG_INFO " Cleanmarker written at 0x%llx.",
					       erase.addr);
			}
		}
	}

	return 0;
}

/**
 * Erase one or more nand blocks.
 * @param mtd - mtd info structure
 * @param image_type - image type, e.g TYPE_UBI, TYPE_YAFFS, TYPE_RAW
 * @param off - offset relative to nand start
 * @param size - size to be erased
 * @param lim - maximum size that actual may be in order to not
 *          exceed the buffer
 * @param spread - if it's 1, erase enough for given file size,
 *          otherwise, 'size' includes skipped bad blocks.
 * @param force - erase all factory set bad blocks if it's true
 *
 * @return 0 on sucess, otherwise 1
 */
static int prog_nand_erase(struct mtd_info *mtd, int image_type,
		loff_t off, loff_t size, loff_t lim, int spread, int force)
{
	int ret = 0;
	nand_erase_options_t opts;
	struct nand_chip *chip = mtd->priv;

	if ((off & (mtd->erasesize - 1)) != 0) {
		PRINT(PROG_ERROR "Attempt to erase non block-aligned data\n");
		return -1;
	}

	if (nand_platform_data.vnand.is_vnand == 1 && force == 0)
		ret = vnand_scan_badblock(mtd, off, size, 0);

	if (ret != 0)
		return ret;

	memset(&opts, 0, sizeof(opts));
	opts.offset = off;
	opts.length = size;
	opts.quiet  = 0;
	opts.spread = spread;
	opts.scrub = force;
	opts.lim = lim;
	if (image_type == TYPE_JFFS2)
		opts.jffs2  = 1;

	ret = prog_nand_erase_opts(mtd, &opts);

	if (force == 1) {
		if (nand_platform_data.vnand.is_vnand == 1)
			ret = vnand_scan_badblock(mtd, off, size, 1);
		else
			chip->scan_bbt(mtd);
	}

	return ret;
}

/**
 * check_skip_len
 *
 * Check if there are any bad blocks, and whether length including bad
 * blocks fits into device
 *
 * @param nand NAND device
 * @param offset offset in flash
 * @param length image length
 * @param used length of flash needed for the requested length
 * @return 0 if the image fits and there are no bad blocks
 *         1 if the image fits, but there are bad blocks
 *        -1 if the image does not fit
 */
static int check_skip_len(nand_info_t *nand, loff_t offset, size_t length,
		size_t *used)
{
	size_t len_excl_bad = 0;
	int ret = 0;

	while (len_excl_bad < length) {
		size_t block_len, block_off;
		loff_t block_start;

		if (offset >= nand->size)
			return -1;

		block_start = offset & ~(loff_t)(nand->erasesize - 1);
		block_off = offset & (nand->erasesize - 1);
		block_len = nand->erasesize - block_off;

		if (!nand_block_isbad(nand, block_start))
			len_excl_bad += block_len;
		else
			ret = 1;

		offset += block_len;
		*used += block_len;
	}

	/* If the length is not a multiple of block_len, adjust. */
	if (len_excl_bad > length)
		*used -= (len_excl_bad - length);

	return ret;
}

#ifdef CONFIG_CMD_NAND_TRIMFFS
static size_t drop_ffs(const nand_info_t *nand, const u_char *buf,
			const size_t *len)
{
	size_t l = *len;
	ssize_t i;

	for (i = l - 1; i >= 0; i--)
		if (buf[i] != 0xFF)
			break;

	/* The resulting length must be aligned to the minimum flash I/O size */
	l = i + 1;
	l = (l + nand->writesize - 1) / nand->writesize;
	l *=  nand->writesize;

	/*
	 * since the input length may be unaligned, prevent access past the end
	 * of the buffer
	 */
	return min(l, *len);
}
#endif

/**
 * prog_nand_write_skip_bad:
 *
 * Write image to NAND flash.
 * Blocks that are marked bad are skipped and the is written to the next
 * block instead as long as the image is short enough to fit even after
 * skipping the bad blocks.  Due to bad blocks we may not be able to
 * perform the requested write.  In the case where the write would
 * extend beyond the end of the NAND device, both length and actual (if
 * not NULL) are set to 0.  In the case where the write would extend
 * beyond the limit we are passed, length is set to 0 and actual is set
 * to the required length.
 *
 * @param nand  	NAND device
 * @param offset	offset in flash
 * @param length	buffer length
 * @param actual	set to size required to write length worth of
 *			buffer or 0 on error, if not NULL
 * @param lim		maximum size that actual may be in order to not
 *			exceed the buffer
 * @param buffer        buffer to read from
 * @param flags		flags modifying the behaviour of the write to NAND
 * @return		0 in case of success
 */
static int prog_nand_write_skip_bad(nand_info_t *nand, loff_t offset,
		size_t *length, size_t *actual, loff_t lim, u_char *buffer,
		int flags)
{
	int rval = 0, blocksize;
	size_t left_to_write = *length;
	size_t used_for_write = 0;
	u_char *p_buffer = buffer;
	int need_skip;

	if (actual)
		*actual = 0;

#ifdef CONFIG_CMD_NAND_YAFFS
	if (flags & WITH_YAFFS_OOB) {
		if (flags & ~WITH_YAFFS_OOB)
			return -EINVAL;

		int pages;
		pages = nand->erasesize / nand->writesize;
		blocksize = (pages * nand->oobsize) + nand->erasesize;
		if (*length % (nand->writesize + nand->oobsize)) {
			PRINT(PROG_ERROR "Attempt to write incomplete page"
				" in yaffs mode\n");
			return -EINVAL;
		}
	} else
#endif
	{
		blocksize = nand->erasesize;
	}

	if ((offset & (nand->writesize - 1)) != 0) {
		PRINT(PROG_ERROR "Attempt to write non page-aligned data\n");
		*length = 0;
		return -EINVAL;
	}

	need_skip = check_skip_len(nand, offset, *length, &used_for_write);

	if (actual)
		*actual = used_for_write;

	if (need_skip < 0) {
		PRINT(PROG_ERROR "Attempt to write outside the flash area\n");
		*length = 0;
		return -EINVAL;
	}

	if (used_for_write > lim) {
		PRINT(PROG_ERROR "Size of write exceeds partition or device"
				"limit\n");
		*length = 0;
		return -EFBIG;
	}

	if (!need_skip && !(flags & WITH_DROP_FFS)) {
		rval = nand_write(nand, offset, length, buffer);
		if (rval == 0)
			return 0;

		*length = 0;
		PRINT(PROG_WARN "NAND write to offset %llx failed %d\n",
			offset, rval);

		return rval;
	}

	while (left_to_write > 0) {
		size_t block_offset = offset & (nand->erasesize - 1);
		size_t write_size, truncated_write_size;

		if (nand_block_isbad(nand, offset & ~(nand->erasesize - 1))) {
			PRINT(PROG_INFO "Skip bad block 0x%08llx\n",
				offset & ~(nand->erasesize - 1));
			offset += nand->erasesize - block_offset;
			continue;
		}

		if (left_to_write < (blocksize - block_offset))
			write_size = left_to_write;
		else
			write_size = blocksize - block_offset;

#ifdef CONFIG_CMD_NAND_YAFFS
		if (flags & WITH_YAFFS_OOB) {
			int page, pages;
			size_t pagesize = nand->writesize;
			size_t pagesize_oob = pagesize + nand->oobsize;
			struct mtd_oob_ops ops;

			ops.len = pagesize;
			ops.ooblen = nand->oobsize;
			ops.mode = MTD_OPS_AUTO_OOB;
			ops.ooboffs = 0;

			pages = write_size / pagesize_oob;
			for (page = 0; page < pages; page++) {

				ops.datbuf = p_buffer;
				ops.oobbuf = ops.datbuf + pagesize;

				rval = mtd_write_oob(nand, offset, &ops);
				if (rval != 0)
					break;

				offset += pagesize;
				p_buffer += pagesize_oob;
			}
		}
		else
#endif
		{
			truncated_write_size = write_size;
#ifdef CONFIG_CMD_NAND_TRIMFFS
			if (flags & WITH_DROP_FFS)
				truncated_write_size = drop_ffs(nand, p_buffer,
						&write_size);
#endif

			rval = nand_write(nand, offset, &truncated_write_size,
					p_buffer);
			offset += write_size;
			p_buffer += write_size;
		}

		if (rval != 0) {
			PRINT(PROG_WARN "NAND write to offset %llx failed %d\n",
				offset, rval);

			*length -= left_to_write;
			return rval;
		}

		left_to_write -= write_size;
	}

	return 0;
}

/**
 * prog_nand_read_skip_bad:
 *
 * Read image from NAND flash.
 * Blocks that are marked bad are skipped and the next block is read
 * instead as long as the image is short enough to fit even after
 * skipping the bad blocks.  Due to bad blocks we may not be able to
 * perform the requested read.  In the case where the read would extend
 * beyond the end of the NAND device, both length and actual (if not
 * NULL) are set to 0.  In the case where the read would extend beyond
 * the limit we are passed, length is set to 0 and actual is set to the
 * required length.
 *
 * @param nand NAND device
 * @param offset offset in flash
 * @param length buffer length, on return holds number of read bytes
 * @param actual set to size required to read length worth of buffer or 0
 * on error, if not NULL
 * @param lim maximum size that actual may be in order to not exceed the
 * buffer
 * @param buffer buffer to write to
 * @return 0 in case of success
 */
static int prog_nand_read_skip_bad(nand_info_t *nand, loff_t offset,
		size_t *length, size_t *actual, loff_t lim, u_char *buffer)
{
	int rval;
	size_t left_to_read = *length;
	size_t used_for_read = 0;
	u_char *p_buffer = buffer;
	int need_skip;

	if ((offset & (nand->writesize - 1)) != 0) {
		PRINT(PROG_ERROR "Attempt to read non page-aligned data\n");
		*length = 0;
		if (actual)
			*actual = 0;
		return -EINVAL;
	}

	need_skip = check_skip_len(nand, offset, *length, &used_for_read);

	if (actual)
		*actual = used_for_read;

	if (need_skip < 0) {
		PRINT(PROG_ERROR "Attempt to read outside the flash area\n");
		*length = 0;
		return -EINVAL;
	}

	if (used_for_read > lim) {
		PRINT(PROG_ERROR "Size of read exceeds partition or device"
				"limit\n");
		*length = 0;
		return -EFBIG;
	}

	if (!need_skip) {
		rval = nand_read(nand, offset, length, buffer);
		if (!rval || rval == -EUCLEAN)
			return 0;

		*length = 0;
		PRINT(PROG_WARN "NAND read from offset %llx failed: %s\n",
			offset, prog_strerror(rval));
		return rval;
	}

	while (left_to_read > 0) {
		size_t block_offset = offset & (nand->erasesize - 1);
		size_t read_length;

		if (nand_block_isbad(nand, offset & ~(nand->erasesize - 1))) {
			PRINT(PROG_INFO "Skipping bad block 0x%08llx\n",
				offset & ~(nand->erasesize - 1));
			offset += nand->erasesize - block_offset;
			continue;
		}

		if (left_to_read < (nand->erasesize - block_offset))
			read_length = left_to_read;
		else
			read_length = nand->erasesize - block_offset;

		rval = nand_read(nand, offset, &read_length, p_buffer);
		if (rval && rval != -EUCLEAN) {
			PRINT(PROG_WARN "NAND read from offset %llx failed: %s\n",
				offset, prog_strerror(rval));
			*length -= left_to_read;
			return rval;
		}

		left_to_read -= read_length;
		offset       += read_length;
		p_buffer     += read_length;
	}

	return 0;
}

static int prog_nand_markbad(struct mtd_info *mtd, loff_t *off,
		loff_t lim)
{
	int ret = 0;

	ret = mtd->_block_markbad(mtd, *off);
	if (ret != 0) {
		PRINT(PROG_ERROR "mark block bad failed:"
				" %s\n", prog_strerror(ret));
	} else if (*off >= lim - mtd->erasesize) {
		PRINT(PROG_ERROR "Finding good blocks exceeds "
				"partition or device limit\n");
		ret = -EFBIG;
	} else {
		*off += mtd->erasesize;
		ret = 0;
	}

	return ret;
}

/**
 * Write buffer to nand.
 * @param mtd - mtd info structure
 * @param image_type - image type, e.g TYPE_UBI, TYPE_YAFFS, TYPE_RAW
 * @param offset - offset relative to nand start to be programmed
 * @param frag_size - size to be write this time
 * @param next_off - the offset of next writing
 * @param lim - maximum size that actual could be in order to not
 *			exceed the partition
 * @param buffer - address of buffer to be written
 * @param image_size - the size of the image to be written
 * @param slavecheck - check whether writing is correct on device side
 * @return 0 on sucess, otherwise 1
 */
static int prog_nand_write(struct mtd_info *mtd, int image_type,
		loff_t offset, size_t frag_size, loff_t *next_off,
		loff_t lim, u_char *buffer, loff_t image_size,
		loff_t written_size, bool slavecheck)
{
	int ret = 0, flags = 0;
	loff_t off = offset;
	u_char *p_buffer = buffer;
	loff_t left_to_write = frag_size;
	size_t actual_size, write_size;
	size_t used_for_write = 0;
	static int complete_percent = 0;
	int need_skip;

	if (buffer == NULL) {
		PRINT(PROG_ERROR "buffer is NULL!\n");
		return -1;
	}

	switch (image_type) {
	case TYPE_YAFFS:
		flags = WITH_YAFFS_OOB;
		break;
	case TYPE_UBI:
		flags |= WITH_DROP_FFS;
		break;
	}

	need_skip = check_skip_len(mtd, off, frag_size, &used_for_write);

	if (need_skip < 0) {
		PRINT(PROG_ERROR "Attempt to write outside the flash partition"
				" area\n");
		return -1;
	}

	/* Write nand by block */
	while (left_to_write > 0) {
		unsigned long long n;
		int percent;
		loff_t actual_off;

		if (left_to_write < mtd->erasesize)
			write_size = left_to_write;
		else
			write_size = mtd->erasesize;

		ret = prog_nand_write_skip_bad(mtd, off, &write_size,
				&actual_size, lim, p_buffer, flags);

		actual_off = off + actual_size - write_size;

		LOGD("off=%#llx actual_off=%#llx buffer %08x write_size %#x\n",
				off, actual_off, *(u32 *)p_buffer, write_size);

		if (ret) {
			/* if writting fail, then mark bad and retry */
			if (ret == -EIO && !nand_check_wp(mtd)) {
				int res = 0;
				PRINT(PROG_WARN "nand write IO error, mark the block(%#llx) "
						"bad, and try next block.\n", actual_off);

				res = prog_nand_markbad(mtd, &actual_off, offset + lim);
				if (res == 0) {
					off = actual_off;
					continue;
				}
			}
			PRINT(PROG_ERROR "NAND write error: %s\n", prog_strerror(ret));
			return -1;
		}

		if (slavecheck) {
			size_t check_size = write_size;

			LOGD("do nand check on device at offset %#llx.\n", actual_off);

			u_char *checkbuf;

			checkbuf = memalign(CONFIG_SYS_CACHELINE_SIZE, mtd->erasesize);

			if (checkbuf == NULL) {
				PRINT(PROG_ERROR "No enough memory for checkbuf.\n");
				return -1;
			}

			/* Read back the block just written, check_size will be
			 * modified to be the size that actually read when reading
			 * failed.
			 */
			ret = prog_nand_read_skip_bad(mtd, actual_off, &check_size,
					NULL, lim, checkbuf);

			if (ret == -EBADMSG) { /* ECC error */
				int res = 0;
				PRINT(PROG_WARN "nand reading for check occurs ECC error,"
						"mark the block(%#llx) bad, and try next "
						"block.\n", actual_off);

				res = prog_nand_markbad(mtd, &actual_off, offset + lim);
				if (res == 0) {
					off = actual_off;
					continue;
				} else
					return -1;
			}

			/* compare the block content just read with orignal data */
			ret = memcmp(p_buffer, checkbuf, write_size);
			if (ret != 0) {
				int res = 0;
				PRINT(PROG_WARN "nand check failed, "
						"mark the block(%#llx) bad, and try next "
						"block.\n", actual_off);
				res = prog_nand_markbad(mtd, &actual_off, offset + lim);
				if (res == 0) {
					off = actual_off;
					continue;
				} else
					return -1;
			} else
				LOGD("checking pass.\n");

			free(checkbuf);
			checkbuf = NULL;
		}

		off = actual_off + mtd->erasesize;
		if (next_off != NULL) {
			*next_off = off;
		}
		p_buffer += mtd->erasesize;

		left_to_write -= write_size;
		written_size += write_size;
		LOGD("actual size = %#x left_to_write %#llx written_size %#llx\n",
				actual_size, left_to_write, written_size);

		n = written_size * 100ULL;
		do_div(n, image_size);
		percent = (int)n;
		if (percent != complete_percent)
			PRINT(PROG_INFO "progress %d, writing at %#llx\n",
					percent, actual_off);
		complete_percent = percent;

		if (percent == 100)
			complete_percent = 0;
	}

	return ret;
}

static int prog_mmc_erase(int devnum, loff_t off, loff_t size,
		loff_t lim)
{
	uint32_t blk_off, blk_cnt, erased_cnt;
	struct mmc *mmc = find_mmc_device(devnum);

	if (!mmc) {
		PRINT(PROG_ERROR "no MSC device at slot %x\n", devnum);
		return -1;
	}

	if (mmc_init(mmc) != 0) {
		PRINT(PROG_ERROR "MSC device init failed!\n");
		return -1;
	}

	if (mmc_getwp(mmc) == 1) {
		PRINT(PROG_ERROR "MMC is write protected!\n");
		return -1;
	}

	if ((size & (mmc->write_bl_len - 1)) != 0) {
		LOGD("%#llx isn't sector aligned, align it.\n", size);
		size += mmc->write_bl_len;
	}

	blk_off = lldiv(off, mmc->write_bl_len);
	blk_cnt = lldiv(size, mmc->write_bl_len);

	erased_cnt = mmc->block_dev.block_erase(devnum, blk_off, blk_cnt);

	if (erased_cnt != blk_cnt) {
		PRINT(PROG_ERROR "error when erasing %d sectors, just %d "
				"sectors was erased!\n", blk_cnt, erased_cnt);
		return -1;
	}
	PRINT(PROG_INFO "progress 100, erasing at %#x\n", blk_off);

	return 0;
}

int prog_mmc_write(int devnum, int image_type,
		loff_t offset, size_t frag_size, loff_t lim, u_char *buffer,
		size_t image_size, bool slavecheck)
{
	uint32_t blk_off;
	u_char *p_buffer = buffer;
	size_t left_to_write;
	static size_t written_size = 0;
	static int complete_percent = 0;
	uint32_t write_unit;

	if (buffer == NULL) {
		PRINT(PROG_ERROR "buffer is NULL!\n");
		return -1;
	}

	struct mmc *mmc = find_mmc_device(devnum);

	if (!mmc) {
		PRINT(PROG_ERROR "no MSC device at slot %x\n", devnum);
		return -1;
	}

	if (mmc_init(mmc) != 0) {
		PRINT(PROG_ERROR "MSC device init failed!\n");
		return -1;
	}

	if (mmc_getwp(mmc) == 1) {
		PRINT(PROG_ERROR "MMC is write protected!\n");
		return -1;
	}

	if ((offset & (mmc->write_bl_len - 1)) != 0) {
		PRINT(PROG_ERROR "Attempt to write non block-aligned data\n");
		return -1;
	}

	if ((frag_size & (mmc->write_bl_len - 1)) != 0) {
		int pad_size = mmc->write_bl_len -
				(frag_size & (mmc->write_bl_len - 1));
		frag_size += pad_size;
		image_size += pad_size;
		memset(p_buffer + frag_size, 0, pad_size);
		LOGD("p_buffer %p %x isn't sector aligned, padding %d bytes.\n",
				p_buffer, frag_size, pad_size);
	}

	blk_off = lldiv(offset, mmc->write_bl_len);
	left_to_write = lldiv(frag_size, mmc->write_bl_len);

	LOGD("blk_off=%d cnt=%d\n", blk_off, left_to_write);

	/*
	 * Use 2048*mmc->write_bl_len as writing unit,
	 * it's 1MB if write_bl_len is 512.
	 **/
	write_unit = 2048;

	/* Write MMC by blocksize */
	while (left_to_write > 0) {
		size_t write_size = 0;
		unsigned long long n;
		int percent;
		uint32_t ret;

		if (left_to_write < write_unit)
			write_size = left_to_write;
		else
			write_size = write_unit;

		LOGD("blk_off=%d  buffer %08x write_size %d\n",
				blk_off, *(u32 *)p_buffer, write_size);

		ret = mmc->block_dev.block_write(devnum, blk_off, write_size,
				p_buffer);
		if (ret != write_size) {
			PRINT(PROG_ERROR "error when writing data(size=%d sectors)"
				" to MMC, just %d sectors was written!\n", write_size,
				ret);
			return -1;
		}

		blk_off += write_unit;
		p_buffer += write_unit * mmc->write_bl_len;

		written_size += write_size;
		left_to_write -= write_size;

		LOGD("left_to_write %d sector(s) written_size %d sector(s)\n",
				left_to_write, written_size);

		n = written_size * mmc->write_bl_len * 100ULL;
		percent = lldiv(n, image_size);
		if (percent != complete_percent)
			PRINT(PROG_INFO "progress %d, writing at %#llx\n", percent,
					(uint64_t)((blk_off - write_unit) * mmc->write_bl_len));
		complete_percent = percent;

		if (percent == 100) {
			written_size = 0;
			complete_percent = 0;
		}
	}

	return 0;
}

static int get_blksz(int image_type)
{
	loff_t blksz;

	if (image_type == TYPE_YAFFS)
		blksz = cache_params[1].blksz;
	else
		blksz = cache_params[0].blksz;

	if (blksz == 0) {
		PRINT(PROG_ERROR "please set proper blksz before programming!\n");
		return -1;
	} else
		LOGD("blksz=%#llx for type %d\n", (unsigned long long)blksz,
				image_type);

	return blksz;
}

static int prog_nand_write_spl(struct mtd_info *mtd, struct spl_params *spl,
		u_char *buf, loff_t *off, loff_t *size, bool slavecheck)
{
	int cnt, ret = 0;

	LOGD("NAND SPL back-up blocks count=%d, SPL size=%d bytes\n",
			(uint32_t)spl->bak_blocks, spl->size);

	if ((uint32_t)size < spl->size) {
		PRINT(PROG_ERROR "bootloader image shouldn't be smaller "
				"than NAND SPL size(%d bytes)\n", spl->size);
		return -1;
	}

	if (buf == NULL) {
		PRINT(PROG_ERROR "buffer is NULL!\n");
		return -1;
	}

	for (cnt = 0; cnt < spl->bak_blocks; cnt++) {
		ret = mtd->_write_user_prot_reg(mtd, cnt * mtd->erasesize,
				(size_t)spl->size, NULL, buf);
		if (ret != 0) {
			PRINT(PROG_ERROR "write spl error: %s\n",
					prog_strerror(ret));
			return -1;
		}
	}

	if (slavecheck) {
		uint8_t *checkbuf = memalign(CONFIG_SYS_CACHELINE_SIZE, spl->size);

		int pass = 0;
		for (cnt = 0; cnt < spl->bak_blocks; cnt++) {
			ret = mtd->_read_user_prot_reg(mtd, cnt * mtd->erasesize,
					spl->size, NULL, checkbuf);

			if ((ret == 0) && !memcmp(buf, checkbuf, spl->size)) {
				LOGD("spl check at %#x OK!\n", cnt * mtd->erasesize);
				pass = 1;
			} else
				PRINT(PROG_WARN "spl check at %#x error!\n",
						cnt * mtd->erasesize);
		}

		free(checkbuf);

		if (pass != 1) {
			PRINT(PROG_ERROR "All spl copies check error!\n");
			return -1;
		}
	}

	*size -= spl->size;
	*off += spl->bak_blocks * mtd->erasesize;
	return 0;
}

/**
 * Host writes buffer to the storage media on device.
 * @param media_type - storage media type, e.g. TYPE_NAND, TYPE_MSC
 * @param image_type - image type, e.g TYPE_UBI, TYPE_YAFFS, TYPE_RAW
 * @param devnum - device index
 * @param off - offset relative to device start to be programmed
 * @param size - size to be write
 * @param lim - maximum size that actual could be in order to not
 *			exceed the partition
 * @param buf - address of buffer to be written
 * @param slavecheck - check whether writing is correct on device side
 *
 * @return 0 on sucess, otherwise 1
 */
static int do_host_program(int media_type, int image_type, int devnum,
		loff_t off, loff_t size, loff_t lim, u_char *buf, bool slavecheck)
{
	size_t len_read;
	int i, count, frag, ret = 0;
	uint64_t tmp, sum = 0;
	loff_t blksz;
	struct rio_dev *riodev = NULL;
	struct mtd_info *mtd = &nand_info[devnum];

	riodev = rio_open("usbtty", 0, 0);
	if (IS_ERR(riodev)) {
		PRINT(PROG_ERROR "get device riodev failed!\n");
		return 1;
	}

	/*
	 * For writting NAND bootloader, write SPL first, so the rest data
	 * could be written by block.
	 */
	if (media_type == TYPE_NAND && off == 0) {
		int ret = 0;
		size_t len_read;
		struct spl_params spl;

		ret = query_nand_spl_params(mtd, &spl);

		if ((uint32_t)size < spl.size) {
			PRINT(PROG_ERROR "bootloader image shouldn't be smaller "
					"than NAND SPL size(%d bytes)\n", spl.size);
			goto err;
		}

		PRINT(PROG_INFO "sub-program-ready\n");
		len_read = rio_read(riodev, buf, spl.size);

		LOGD("first dword in offset 0 is %016llx\n", *(uint64_t *)buf);
		LOGD("first dword in offset 0x2000 is %016llx\n",
				*(uint64_t *)(buf + 0x2000));

		if (len_read < spl.size) {
			PRINT(PROG_ERROR "error when reading data(size=%#xB)"
					" from host, just %#xB was read!\n",
					spl.size, len_read);
			return -1;
		}

		ret = prog_nand_write_spl(mtd, &spl, buf, &off, &size, slavecheck);
		if (ret < 0)
			goto err;
	}

	LOGD("getting data from host...\n");
	blksz = get_blksz(image_type);
	if (blksz < 0)
		goto err;

	tmp = size;
	frag = do_div(tmp, (ulong)blksz); /* FIXME */
	count = tmp + !!frag;
	LOGD("transfer count=%d blksz=%#llx frag=%#x\n",
			count, (unsigned long long)blksz, frag);

	for (i = 0; i < count; i++) {
		size_t len_to_read =
				(frag != 0 && (i == count - 1)) ? frag : blksz;
		LOGD("len_to_read=%#x, pbuf=%p, index=%d\n",
				len_to_read, buf, i);

		PRINT(PROG_INFO "sub-program-ready\n");
		len_read = rio_read(riodev, buf, len_to_read);

		if (IS_ERR_VALUE(len_read)) {
			PRINT(PROG_ERROR "Error occurred when do robust udc reading:"
					" %s\n", prog_strerror(len_read));
			goto err;
		}
		LOGD("first dword in buffer is %016llx\n", *(uint64_t *)buf);

		if (len_read < len_to_read) {
			PRINT(PROG_ERROR "error when reading data(size=%#xB)"
					" from host, just %#xB was read!\n",
					len_to_read, len_read);
			goto err;
		}

		switch (media_type) {
		case TYPE_NAND: {
				loff_t next_off;
				ret = prog_nand_write(mtd, image_type, off,
						len_read, &next_off, lim, buf, size, sum,
						slavecheck);
				off = next_off;
				break;
			}
		case TYPE_MSC:
			ret = prog_mmc_write(devnum, image_type, off + sum,
					len_read, lim, buf, size, slavecheck);
			break;
		case TYPE_MEM:
			memcpy((void *)(uint32_t)(off + sum), buf, len_read);
			break;
		}

		sum += len_read;

		if (ret != 0)
			goto err;
	}

	rio_close(riodev);
	return 0;

err:
	rio_close(riodev);
	return 1;
}

static int str2type(char *str)
{
	int image_type = 0;

	if (!strcmp(str, "raw"))
		image_type = TYPE_RAW;
	else if (!strcmp(str, "ubi"))
		image_type = TYPE_UBI;
	else if (!strcmp(str, "yaffs"))
		image_type = TYPE_YAFFS;
	else if (!strcmp(str, "vnand"))
		image_type = TYPE_VNAND;
	else if (!strcmp(str, "jffs2"))
		image_type = TYPE_JFFS2;
	else if (!strcmp(str, "ext4"))
		image_type = TYPE_EXT4;
	else {
		PRINT(PROG_ERROR "Image type %s is not supported!\n", str);
		image_type = -1;
	}

	return image_type;
}

static void set_default_cache_params(struct mtd_info *mtd)
{
	static bool done = false;

	if (!done) {
		/* prepare parameters for caching images */
		cache_params[0].min = cache_params[0].step = 1 * 1024 * 1024;
		cache_params[0].max = misc_param.malloc_len;
		cache_params[0].blksz = cache_params[0].min * 16;

		/* parameters for yaffs */
		cache_params[1].min = cache_params[1].step =
				(mtd->writesize + mtd->oobsize) *
				(mtd->erasesize / mtd->writesize);
		cache_params[1].max = (mtd->writesize + mtd->oobsize) *
				(misc_param.malloc_len / (mtd->writesize + mtd->oobsize));
		cache_params[1].blksz = cache_params[1].min * 16;

		done = true;
	}
}

static int mmc_name_to_devid(char *name)
{
	int i, mmc_num;
	struct mmc *m;

	mmc_num = get_mmc_num();
	for (i = 0; i < mmc_num; i++) {
		if ((m = find_mmc_device(i)) != NULL) {
			if (!strcmp(m->name, name))
				break;
		}
	}
	if (i == mmc_num)
		return -1;
	else
		return i;
}

/**
 * Routine implementing programmer command which does programming related
 * work.
 *
 * @param cmdtp - command internal data
 * @param flag - command flag
 * @param argc - number of arguments supplied to the command
 * @param argv - arguments list
 * @return 0 on success, 1 otherwise
 */
static int do_programmer(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	char *cmd, *dev;
	int dev_idx = 0, dev_idx1 = 0;
	loff_t off;
	loff_t size, size1 = 0;
	loff_t maxsize;
	ulong addr = 0; /* for card-burn */
	int ret = 0;
	struct mtd_info *mtd = &nand_info[0];
	u_char *buf = NULL;

	set_default_cache_params(mtd);

	if (argc < 3)
		goto usage;

	/* argv[1] stores command for query and set */
	cmd = argv[1];

	if (!strcmp(cmd, "query")) {
		/* query the memory size for caching images in device */
		if (!strcmp(argv[2], "blksz")) {
			int index;
			int image_type;

			if (argc < 4)
				goto usage;

			image_type = str2type(argv[3]);
			if (image_type < 0)
				goto fail;

			index = (image_type == TYPE_YAFFS)? 1 : 0;
			if (argc == 4) {
				PRINT(PROG_INFO "%#llx,%#llx,%#llx,%#llx\n",
						cache_params[index].min,
						cache_params[index].step,
						cache_params[index].max,
						cache_params[index].blksz);
			} else if (argc == 5 && str2long(argv[4], &addr)) {
				/* for card-burn */
				LOGD("for card-burn, addr = %#x\n", (unsigned int)addr);
				if ((addr & 0x3) != 0 || addr == 0) {
					LOGE("address error！\n");
					goto fail;
				}
				*(ulong *) addr = cache_params[index].max;
			} else
				goto usage;
			goto finish;
		} else if (!strcmp(argv[2], "device")) {
			query_device();
			goto finish;
		} else if (!strncmp(argv[2], "nand", 4)) {
			if (argc != 4)
				goto usage;

			if (strict_strtoul(argv[2] + 4, 10, (unsigned long *)&dev_idx)) {
				PRINT(PROG_ERROR "nand device id is not a number! \n");
				goto fail;
			} else
				LOGD("nand id is %d\n", dev_idx);

			if (dev_idx < CONFIG_SYS_MAX_NAND_DEVICE)
				mtd = &nand_info[dev_idx];
			else {
				PRINT(PROG_ERROR "nand device id exceed max number!\n");
				goto fail;
			}

			if (!strcmp(argv[3], "part"))
				ret = query_nand_part(mtd);
			else if (!strcmp(argv[3], "spl.params")) {
				struct spl_params spl;
				ret = query_nand_spl_params(mtd, &spl);
			} else if (!strcmp(argv[3], "spl.erasecount"))
				ret = query_nand_spl_erasecount(mtd, NULL);
			else
				goto usage;

			if (ret == 0)
				goto finish;
			else
				goto fail;
		} else if(!strncmp(argv[2], "msc", 3)) {
			PRINT(PROG_ERROR "msc is not supported now!\n");
			goto fail;
		} else
			goto usage;
	}

	if (!strcmp(cmd, "set")) {
		loff_t tmp;
		int image_type;

		if (argc < 5)
			goto usage;
		image_type = str2type(argv[3]);
		if (image_type < 0)
			goto fail;

		if (argc == 5 && !strcmp(argv[2], "blksz") &&
				str2off(argv[4], &tmp)) {
			if (set_blksz(image_type, tmp) == 0)
				goto finish;
			else
				goto fail;
		} else
			goto usage;
	}

	/* argv[2] stores command for erase, write and read */
	cmd = argv[2];
	dev = argv[1];

	if (!strncmp(cmd, "erase", 5)) {
		if (argc < 4 || argc > 7)
			goto usage;

		if (!strncmp("nand", dev, 4)) {
			bool is_part; /* whether use partition name as parameter */
			int force = !strcmp(&cmd[5], ".force");
			int image_type = TYPE_RAW;
			int spread = 0;

			if (cmd[5] != '\0') {
				if (strcmp(&cmd[5], ".force") != 0) {
					PRINT(PROG_ERROR "erase option %s isn't supported!\n",
							&cmd[5]);
					goto usage;
				}
			}

			if (strict_strtoul(dev + 4, 10, (unsigned long *)&dev_idx)) {
				PRINT(PROG_ERROR "nand device id is not a number! \n");
				goto fail;
			}

			if (arg_off(argv[3], &dev_idx1, &off, &size1, &maxsize,
					&is_part))
				goto fail;

			if (!is_part && argc >= 5) {
				size = (int) simple_strtoul(argv[4], NULL, 16);
				if (size > size1) {
					PRINT(PROG_ERROR "exceed patition size!\n");
					goto fail;
				}
			} else {
				size = size1;
			}
			LOGD("off = %llx  size = %llx \n",
					(unsigned long long)off, (unsigned long long)size);
			if (dev_idx != dev_idx1) {
				PRINT(PROG_ERROR "partition exceed the specified device!\n");
				goto fail;
			}

			if (is_part) {
				/* the case of using partition name as parameter */
				if (argc == 5) {
					if (!strcmp(argv[4], "spread")) {
						PRINT(PROG_ERROR "don't support spread option"
								"for partition operation.\n");
						goto usage;
					}
					image_type= str2type(argv[4]);
					if (image_type < 0)
						goto usage;
				}
				if (argc > 5)
					goto usage;
			} else {
				/* the case of using 'off' and 'size' as parameter */
				if (argc == 6) {
					if (!strcmp(argv[5], "spread"))
						spread = 1;
					else {
						image_type=str2type(argv[5]);
						if (image_type < 0)
								goto usage;
					}
				} else if (argc == 7) {
					image_type = str2type(argv[5]);
					if (image_type < 0)
						goto usage;
					if (!strcmp(argv[6], "spread"))
						spread = 1;
					else
						goto usage;
				}
			}

			LOGD("image_type %d off %#llx size %#llx maxsize %#llx"
					" is_part %d spread %d force %d\n", image_type,
					off, size, maxsize, is_part, spread, force);

			mtd = &nand_info[dev_idx];
			ret = prog_nand_erase(mtd, image_type, off, size, maxsize,
					spread, force);

			if (ret == 0)
				goto finish;
			else {
				PRINT(PROG_ERROR "NAND Erase failure!\n");
				goto fail;
			}
		} else if (!strncmp("msc", dev, 3)) {
			char *tail;

			dev_idx = mmc_name_to_devid(dev);
			if (dev_idx < 0) {
				PRINT(PROG_ERROR "No such msc device!\n");
				goto fail;
			}

			off = simple_strtoull(argv[3], &tail, 16);
			if (tail == argv[3]) {
				PRINT(PROG_ERROR "off %s is not a valid number!\n",
						argv[3]);
				goto fail;
			}

			size = simple_strtoull(argv[4], &tail, 16);
			if (tail == argv[4]) {
				PRINT(PROG_ERROR "size %s is not a valid number!\n",
						argv[4]);
				goto fail;
			}

			LOGD("off = %#llx  size = %#llx \n",
					(unsigned long long)off, (unsigned long long)size);

			ret = prog_mmc_erase(dev_idx, off, size, 0);

			if (ret == 0)
				goto finish;
			else
				goto fail;
		} else {
			goto usage;
		}
	}

	if (!strncmp(cmd, "write", 5)) {
		bool slavecheck = false;
		int image_type;
		loff_t blksz;
		u_char *card_buf = NULL;
		bool is_cardburn = argc >= 7 && str2long(argv[6], &addr);

		if (argc < 6)
			goto usage;

		image_type = str2type(argv[5]);
		LOGD("image type %s(%d)\n", argv[5], image_type);
		if (image_type < 0) {
			PRINT(PROG_ERROR "image type %s isn't supported!\n",
					argv[5]);
			goto fail;
		}

		blksz = get_blksz(image_type);

		if (blksz < 0)
			goto fail;
		if (!is_cardburn) {
			buf = memalign(CONFIG_SYS_CACHELINE_SIZE, blksz);
			if (buf == NULL ) {
				PRINT(PROG_ERROR "could not allocate enough memory, "
						"please set smaller blksz!\n");
				goto fail;
			} else
				LOGD("buf addr=%p\n", buf);
		} else
			card_buf = (u_char *)addr;

		if (cmd[5] != '\0') {
			if (!strcmp(&cmd[5], ".slavecheck"))
				slavecheck = true;
			else {
				PRINT(PROG_ERROR "write option %s isn't supported!\n",
						&cmd[5]);
				goto usage;
			}
		}

		if (!strncmp("nand", dev, 4)) {
			if (strict_strtoul(dev + 4, 10, (unsigned long *) &dev_idx)) {
				PRINT(PROG_ERROR "nand device id is not a number! \n");
				goto fail;
			}
			if (arg_off_size(argc - 3, argv + 3, &dev_idx1, &off, &size1,
					&maxsize, NULL))
				goto fail;
			size = (int) simple_strtoul(argv[4], NULL, 16);
			if (size > size1) {
				PRINT(PROG_ERROR "exceed patition size!\n");
				goto fail;
			}

			LOGD("off = %llx  size = %llx \n",
					(unsigned long long)off, (unsigned long long)size);
			if (dev_idx != dev_idx1) {
				PRINT(PROG_ERROR "partition exceed the specified device!\n");
				goto fail;
			}

			mtd = &nand_info[dev_idx];
			if (is_cardburn) {
				LOGD("card burn ...\n");
				if (off == 0) {
					struct spl_params spl;
					LOGD("first dword in offset 0 is %016llx\n",
							*(uint64_t *)card_buf);
					LOGD("first dword in offset 0x2000 is %016llx\n",
							*(uint64_t *)(card_buf + 0x2000));

					query_nand_spl_params(mtd, &spl);
					prog_nand_write_spl(mtd, &spl, card_buf, &off, &size,
							slavecheck);

					card_buf += spl.size;
				}

				ret = prog_nand_write(mtd, image_type, off, size,
						NULL, maxsize, card_buf, size, 0,
						slavecheck);
				if (ret == 0)
					goto finish;
				else
					goto fail;
			}

			LOGD("slavecheck %d\n", slavecheck);
			ret = do_host_program(TYPE_NAND, image_type, dev_idx, off,
					size, maxsize, buf, slavecheck);

			if (ret != 0) {
				PRINT(PROG_ERROR "NAND Write failure!\n");
				goto fail;
			}

			if (argc >= 7) {
				/* TODO: if need hostcheck, check on host */
				LOGD("need hostcheck or reset? argc=%d\n", argc);
				if (!strcmp(argv[6], "hostcheck")) {
					PRINT(PROG_ERROR "hostcheck is not supported!\n");
					goto fail;
				}

				/* echo READY before reset */
				PRINT(PROG_INFO PROG_READY);
				/* if need reset，reset device */
				if (!strcmp(argv[6], "reset"))
					run_command("reset", 0);
				if ((argc == 8) && !strcmp(argv[7], "reset"))
					run_command("reset", 0);
			}

			if (ret == 0)
				goto finish;
			else
				goto fail;

		} else if (!strncmp("msc", dev, 3)) {
			char *tail;

			dev_idx = mmc_name_to_devid(dev);
			if (dev_idx < 0) {
				PRINT(PROG_ERROR "No such msc device!\n");
				goto fail;
			}

			off = simple_strtoull(argv[3], &tail, 16);
			if (tail == argv[3]) {
				PRINT(PROG_ERROR "off %s is not a valid number!\n",
						argv[3]);
				goto fail;
			}

			size = simple_strtoull(argv[4], &tail, 16);
			if (tail == argv[4]) {
				PRINT(PROG_ERROR "size %s is not a valid number!\n",
						argv[4]);
				goto fail;
			}

			LOGD("off = %#llx  size = %#llx \n",
					(unsigned long long)off, (unsigned long long)size);

			if (is_cardburn) {
				LOGD("card burn ...\n");
				LOGD("first dword in offset 0x200 is %016llx\n",
						*(uint64_t *)(card_buf + 0x200));
				LOGD("first dword in offset 0x4000 is %016llx\n",
						*(uint64_t *)(card_buf + 0x4000));
				ret = prog_mmc_write(dev_idx, image_type, off,
						size, 0, card_buf, size, slavecheck);
				if (ret == 0)
					goto finish;
				else
					goto fail;
			}

			ret = do_host_program(TYPE_MSC, image_type, dev_idx, off,
					size, maxsize, buf, slavecheck);

			if (ret == 0)
				goto finish;
			else {
				PRINT(PROG_ERROR "MSC Write failure!\n");
				goto fail;
			}
		} else if (!strcmp("mem", dev)) {
			PRINT(PROG_ERROR " mem is not supported now!\n");
			goto fail;
		}
	}

	if (!strcmp(cmd, "read")) {

		PRINT(PROG_ERROR " read command is not supported now!\n");
		goto fail;
	}

	PRINT("The command isn't supported.\n");
	goto usage;

finish:
	if (buf != NULL) {
		free(buf);
		buf = NULL;
	}

	PRINT(PROG_INFO PROG_READY);
	return CMD_RET_SUCCESS;
fail:
	if (buf != NULL) {
		free(buf);
		buf = NULL;
	}

	PRINT(PROG_INFO PROG_READY);
	return CMD_RET_FAILURE;

usage:
	PRINT(PROG_INFO PROG_READY);
	return CMD_RET_USAGE;
}

#ifdef CONFIG_SYS_LONGHELP
static char programmer_help_text[] =
"programmer query device\n"
"  - query the storage media in device.\n"
"programmer query nandX|mscX part\n"
"  - query the partition information of the target storage media.\n"
"programmer query nandX spl.params\n"
"  - query NAND SPL's parameters, including:\n"
"     #buswidth,#toggle,#rowcycle,#pagesize,#splsize,#baknum\n"
"programmer query nandX spl.erasecount\n"
"  - query the erase count of NAND SPL region, in order to be informed\n"
"    of whether it is a new NAND and how old it is.\n"
"programmer query blksz #image_type [#addr]\n"
"  - query the memory size for caching images in device, reporting:\n"
"    #min,#step,#max,#current_value\n"
"    #image_type could be raw, ubi, vnand, yaffs and jffs2.\n"
"    #min,#step,#max,#current_value(4 words) will return to the address\n"
"    specified by parameter '#addr' for card-burn.\n"
"programmer set blksz #image_type #value\n"
"  - host set blksz to #value for caching image_type images\n"
"    #image_type could be raw, ubi, vnand, yaffs and jffs2.\n"
"programmer nandX|mscX erase[.force] #partition|chip [#image_type]\n"
"  - erase entire partition or chip, with '.force',\n"
"   really clean NAND erasing bad blocks (UNSAFE)\n"
"   #image_type could be jffs2, else means nothing\n"
"programmer nandX|mscX erase[.force] #off #size [#image_type] [spread]\n"
"  - erase '#size' bytes from offset '#off'\n"
"    With 'spread', erase enough for given file size, otherwise,\n"
"      '#size' includes skipped bad blocks. It's meanless with .force.\n"
"    #image_type could be jffs2, else means nothing.\n"
"programmer nandX|mscX|mem write[.slavecheck] #off|#partition #size "
                   "#image_type [#addr] [hostcheck] [reset]\n"
"  - #image_type could be raw, ubi, vnand, yaffs and jffs2.\n"
"    write '#size' bytes starting at offset '#off' or #partition\n"
"    with '.slavecheck', check on device while writing;\n"
"    with 'hostcheck', check on host when writting done;\n"
"    with 'reset', reset the device at the end.\n"
"";
#endif

U_BOOT_CMD(programmer, 8, 0, do_programmer, "do programmer",
		programmer_help_text);
