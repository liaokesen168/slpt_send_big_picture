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

#include <config.h>
#include <common.h>
#include <view.h>
#include <picture.h>
#include <linux/string.h>
#include <slpt.h>
#include <slpt_app.h>
#include <slpt_clock.h>

struct slpt_method {
	const char *name;
	unsigned long addr;
};

#define SLPT_METHOD(method)												\
    __attribute__ ((__used__, __section__(".slpt_method_def_list")))	\
    static struct slpt_method method_##method = {						\
        .name = #method,												\
        .addr = (unsigned long)method,									\
    }

extern unsigned long __slpt_method_def_list_start;
extern unsigned long __slpt_method_def_list_end;

int slpt_method_init_onetime(void) {
	int ret;
	struct slpt_method *method;
	struct slpt_method *method_end;

	method = (struct slpt_method *)&__slpt_method_def_list_start;
	method_end = (struct slpt_method *)&__slpt_method_def_list_end;

	for ( ; method < method_end; method++) {
		ret = slpt_kernel_register_method(method->name, method->addr);
		if (ret) {
			pr_err("%s: add [%s] 0x%08lx failed\n", __func__, method->name, method->addr);
		}
	}

	return 0;
}
SLPT_APP_INIT_ONETIME(slpt_method_init_onetime);

int slpt_method_exit_onetime(void) {
	struct slpt_method *method;
	struct slpt_method *method_end;

	method = (struct slpt_method *)&__slpt_method_def_list_start;
	method_end = (struct slpt_method *)&__slpt_method_def_list_end;

	for ( ; method < method_end; method++) {
		slpt_kernel_unregister_method(method->name);
	}

	return 0;
}
SLPT_APP_EXIT_ONETIME(slpt_method_exit_onetime);

#ifdef CONFIG_VIEW
/**
 * method_create_view() - create a view in slpt
 * @argv[0] - view_type : view_type in view_utils.c, like "time-view"
 * @argv[1] - view_name : the view's name
 *
 * @return_val: return 0 if success, nagative value if failed.
 */
int method_create_view(int argc, char *argv[]) {

	struct view *v;
	struct slpt_app_res *clock_dir;
	char *view_type, *view_name;

	if(argc != 2) {
		pr_err("error: your param is error!\n");
		return -1;
	}

	clock_dir = slpt_kernel_name_to_app_res("clock", uboot_slpt_task);
	assert(clock_dir);

	/* argv[0] is the view_type, argv[1] is view_name */
	view_type = argv[0];
	view_name = argv[1];
	v = alloc_view_by_str(view_name, view_type);
	if(v == NULL) {
		pr_err("error: malloc view error.\n");
		return -1;
	}

	if (!slpt_register_view(v, clock_dir, NULL, 0)) {
		slpt_kernel_printf("failed to register clock\n");
		goto slpt_register_view_error;
	}

	slpt_display_add_view(v);

	return 0;

slpt_register_view_error:
	free_view(v);
	return -EINVAL;
}
SLPT_METHOD(method_create_view);
#endif

/**
 * method_create_picture() - create one picture in slpt
 * @argv[0] - grp_name : the grp which the picture will be add in, if grp is not exist, it will be created
 * @argv[1] - pic_name : the picture's name
 * @argv[2] - size     : the picture's size
 *
 * @return_val: return 0 if success, nagative value if failed.
 */
int method_create_picture(int argc, char *argv[]) {
	char *grp_name;
	char *pic_name;
	unsigned int size;

	if (argc != 3) {
		pr_err("%s: Invalid args\n", __func__);
		return -EINVAL;
	}

	grp_name = argv[0];
	pic_name = argv[1];
	size = simple_strtoul(argv[2], NULL, 10);

	pr_info("%s %s %s %d\n", __func__, grp_name, pic_name, size);

	return alloc_picture2(grp_name, pic_name, size) ? 0 : -EINVAL;
}
SLPT_METHOD(method_create_picture);

/**
 * method_create_picture_grp() - create a picture group in slpt, it may contains a lot of pictures
 * @argv[0]     - grp_name           : the grp's name, if the grp is already exist, it will be failed
 * @argv[1...n] - pic_name_with_size : the style is "pic_name:size", the pictrues will be add into the new grp.
 *
 * if the target grp is already exist, it will be failed, and the pictures will not be added into the grp.
 * if 2 or more pictures have the same pic_name, all the pictures and the grp can not be created.
 *
 * @return_val: return 0 if success, nagative value if failed.
 */
int method_create_picture_grp(int argc, char *argv[]) {
	int ret = 0;
	unsigned int i;
	unsigned int size;
	char *grp_name;
	struct picture_desc *descs;

	if (argc < 1) {
		pr_err("%s: Invalid args\n", __func__);
		return -EINVAL;
	}

	grp_name = argv[0];
	size = argc - 1;
	if (size) {
		descs = malloc(size * sizeof(*descs));
		if (!descs) {
			pr_err("%s: failed to alloc picture descs\n", __func__);
			return -ENOMEM;
		}
	} else {
		descs = NULL;
	}

	argv = argv + 1;
	for (i = 0; i < size; ++i) {
		char *p = strchr(argv[i], ':');
		if (!p || p[0] == '\0') {
			pr_err("%s: picture desc is not valid: [%s]\n", __func__, argv[i]);
			ret = -EINVAL;
			goto free_descs;
		}
		p[0] = '\0';
		descs[i].name = argv[i];
		descs[i].size = simple_strtoul(p + 1, NULL, 10);
		pr_info("%s %s %s %d\n", __func__, grp_name, descs[i].name, descs[i].size);
	}

	if (!alloc_picture_grp(grp_name, descs, size)) {
		pr_err("%s: failed to alloc picture grp\n", __func__);
		ret = -ENOMEM;
	}

free_descs:
	free(descs);
	return ret;
}
SLPT_METHOD(method_create_picture_grp);

/**
 * method_free_picture_grp() - free a picture grp (and the pictures in it) in slpt
 * @argv[0] - grp_name: the grp's name, if the grp is not exist, it will be failed
 *
 * @return_val: return 0 if success, nagative value if failed. 
 */
int method_free_picture_grp(int argc, char *argv[]) {
	char *grp_name;

	if (argc != 1) {
		pr_err("%s: Invalid args\n", __func__);
		return -EINVAL;		
	}

	grp_name = argv[0];

	return free_picture_grp_by_name(grp_name);
}
SLPT_METHOD(method_free_picture_grp);

/**
 * method_free_all_picture_grp() - free all picture grp in slpt
 *
 * @return_val: return 0 if success, nagative value if failed.
 */
int method_free_all_picture_grp(int argc, char *argv[]) {
	free_all_picture_grp();
	return 0;
}
SLPT_METHOD(method_free_all_picture_grp);

#ifdef CONFIG_VIEW
/* delete all the view, the default and we alloc create */
static int method_delete_all_views(int argc, char *argv[])
{
	slpt_display_delete_all_view();
	return 0;
}
SLPT_METHOD(method_delete_all_views);
#endif
