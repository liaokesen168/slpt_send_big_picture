/*
 *  Copyright (C) 2015 Wu Jiao <jiao.wu@ingenic.com wujiaososo@qq.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>
#include <slpt.h>
#include <slpt_app.h>
#include <sview/sview.h>
#include <key_reader.h>
#include <slpt_app_low_voltage_detect.h>

#define SLPT_IOCTL_INIT_SVIEW         _IOW('s', 0x121, int)
#define SLPT_IOCTL_CLEAR_PICTURE_GRP  _IOW('s', 0x122, int)
#define SLPT_IOCTL_ADD_PICTURE_GRP    _IOW('s', 0x123, int)
#define SLPT_IOCTL_ADD_PICTURE        _IOW('s', 0x124, int)
#define SLPT_IOCTL_SET_TIME           _IOW('s', 0x125, int)
#define SLPT_IOCTL_SET_LOW_VOL_WARN   _IOW('s', 0x126, int)

struct data_init_sview {
	unsigned int size;
	char mem[];
};

/**
 * slpt_ioctl_init_sview() - init sview in slpt
 *
 * @return_val: return 0 if success, nagative value if failed.
 */
int slpt_ioctl_init_sview(void *hdr, unsigned hdr_len, void *mem, unsigned int mem_len) {
	struct data_init_sview *data = mem;
	struct key_reader *reader;
	struct sview *root_view;
	unsigned int size = mem_len;

	if (data == NULL)
		return -EINVAL;

	reader = alloc_key_reader(mem, size);

	root_sview_free();
	sview_reset_id_counter();

	print_all_picture_grp();

	root_view = create_sview_from_key_reader(reader);
	if (root_view)
		set_root_sview(root_view);

	free_key_reader(reader);

	return 0;
}

static struct picture_grp *cur_grp = NULL;

int slpt_ioctl_clear_picture_grp(void *hdr, unsigned hdr_len, void *mem, unsigned int mem_len) {
	free_all_picture_grp();
	cur_grp = NULL;

	return 0;
}

int slpt_ioctl_add_picture_grp(void *hdr, unsigned hdr_len, void *mem, unsigned int mem_len) {
	const char *grp_name = mem;

	if (grp_name == NULL)
		return -EINVAL;

	cur_grp = alloc_picture_grp(grp_name, NULL, 0);

	return cur_grp != NULL ? 0 : -EINVAL;
}

int slpt_ioctl_add_picture(void *hdr, unsigned hdr_len, void *mem, unsigned int mem_len) {
	const char *pic_name = hdr;
	struct picture_header *header = mem;
	struct picture *pic;
	int ret;

	if (cur_grp == NULL)
		return -ENODEV;

	if (pic_name == NULL || header == NULL)
		return -EINVAL;

	ret = check_picture_header(header);
	if (ret)
		return ret;

	pic = alloc_picture_to_grp(cur_grp, pic_name, header->len);
	if (!pic)
		return -ENOMEM;

	memcpy(pic->buffer, header, header->len);

	return 0;
}

extern void set_last_slpt_time(unsigned long tv_sec, unsigned long tv_usec);

int slpt_ioctl_set_time(void *hdr, unsigned hdr_len, void *mem, unsigned int mem_len) {
	unsigned long *tv_sec = hdr;
	unsigned long *tv_usec = mem;

	set_last_slpt_time(*tv_sec, *tv_usec);

	return 0;
}

int slpt_ioctl_set_low_vol_warn(void *hdr, unsigned hdr_len, void *mem, unsigned int mem_len) {
	int *min_vol = hdr;
	int *max_vol = mem;

	set_low_pmu_voltage_detect_state(*min_vol, *max_vol);

	return 0;
}

/**
 * slpt_ioctl() - slpt ioctl entry
 *
 * if you hava a new ioctl, add your function here, call it in a new switch-case
 * @return_val: return 0 if success, nagative value if failed.
 */
int slpt_ioctl(void *hdr, unsigned hdr_len, void *mem, unsigned int mem_len, unsigned int cmd) {
	int ret = 0;

	switch (cmd) {
	case SLPT_IOCTL_INIT_SVIEW:
		ret = slpt_ioctl_init_sview(hdr, hdr_len, mem, mem_len); break;
	case SLPT_IOCTL_CLEAR_PICTURE_GRP:
		ret = slpt_ioctl_clear_picture_grp(hdr, hdr_len, mem, mem_len); break;
	case SLPT_IOCTL_ADD_PICTURE_GRP:
		ret = slpt_ioctl_add_picture_grp(hdr, hdr_len, mem, mem_len); break;
	case SLPT_IOCTL_ADD_PICTURE:
		ret = slpt_ioctl_add_picture(hdr, hdr_len, mem, mem_len); break;
	case SLPT_IOCTL_SET_TIME:
		ret = slpt_ioctl_set_time(hdr, hdr_len, mem, mem_len); break;
	case SLPT_IOCTL_SET_LOW_VOL_WARN:
		ret = slpt_ioctl_set_low_vol_warn(hdr, hdr_len, mem, mem_len); break;
	default:
		ret = -ENODEV; break;
	}

	return ret;
}

int slpt_ioctl_init_onetime(void) {
	slpt_set_ioctl((void *)slpt_ioctl);
	return 0;
}
SLPT_APP_INIT_ONETIME(slpt_ioctl_init_onetime);

void slpt_ioctl_exit_onetime(void) {
	slpt_set_ioctl(NULL);
}
SLPT_APP_EXIT_ONETIME(slpt_ioctl_exit_onetime);
