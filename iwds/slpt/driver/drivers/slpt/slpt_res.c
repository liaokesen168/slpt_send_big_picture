/*
 *  Copyright (C) 2014 Fighter Sun <wanmyqawdr@126.com>
 *  Copyright (C) 2014 Wu Jiao <jwu@ingenic.cn>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/list.h>
#include <linux/string.h>
#include <linux/slpt.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/highmem.h>
#include <linux/suspend.h>

#include <linux/slpt_battery.h>

#if 0
enum slpt_res_type {
	SLPT_RES_FUNCTION = 0,
	SLPT_RES_INT,

	/* keep last */
	SLPT_RES_MEM,
	SLPT_RES_MAX,
};

struct slpt_app_res {
	const char *name;
	unsigned int type;
	void *addr;
	unsigned int length;
};

struct slpt_res {
	struct slpt_app_res res;
	struct kobject kobj;
	struct slpt_task *task;
	struct list_head link;
};
#endif

LIST_HEAD(slpt_res_handlers);
DEFINE_MUTEX(slpt_res_lock);
extern struct kobject *slpt_res_kobj;
extern struct kobj_type slpt_kobj_ktype;

static int slpt_create_res_sysfs_files(struct slpt_res *sr, struct slpt_task *task);
static void slpt_remove_res_sysfs_files(struct slpt_res *sr, struct slpt_task *task);

static inline void slpt_res_write_int(struct slpt_res *sr, int value) {
	  (*(volatile int *)(sr)->res.addr) = value;
}

static inline int slpt_res_read_int(struct slpt_res *sr) {
	return *(volatile int *)(sr)->res.addr;
}

/**
 * slpt_register_res() - register a slpt resouces
 *
 * @res->name:   resuorce name, is a unique identification for the resource
 * @res->type:   resource type
 * @res->addr:   the addr to store resource data's pointer)
 * @res->length: length of resource data in byte
 * @create_sys:  0: do not create sysfs file for this resource, 1: create the entry
 * @task:        NULL: resource tied to slpt_res_handlers (global),
 *               not NULL: resource tied to @task->res_handlers (task),
 *
 * example:
 *    1, name and task
 *       code:
 *        res->name = "A", slpt_register_res(res, 1, task1); // no "A" in task1, so ok
 *        res->name = "A", slpt_register_res(res, 1, task2); // no "A" in task2, so ok
 *        res->name = "A", slpt_register_res(res, 1, NULL); // no "A" in global, so ok
 *        res->name = "A", slpt_register_res(res, 0, NULL); // already "A" in global, so error
 *        res->name = "B", slpt_register_res(res, 0, NULL); // no "B" in global, so ok
 *       dir:
 *        sys/slpt/res/A
 *        sys/slpt/apps/task1/A
 *        sys/slpt/apps/task2/A
 *
 *    2, register or require a resource
 *       require:
 *         task1:
 *             my_mem = NULL, res->type = SLPT_RES_MEM, res->addr = my_mem;
 *             slpt_register_res(...);
 *             ........
 *             if (my_mem != NULL) do something
 *       register:
 *         task1:
 *             my_mem = alloc_some_res(), res->type = SLPT_RES_MEM, res->addr = my_mem;
 *             slpt_register_res(...);
 *         task2:
 *             res = name_to_slpt_app_res(...);
 *             if (res->addr != NULL) do something
 *
 *
 */
struct slpt_res *slpt_register_res(struct slpt_app_res *res, int create_sys, struct slpt_task *task) {
	struct slpt_res *sr = NULL;
	struct list_head *pos, *handlers;
	int ret;

	mutex_lock(&slpt_res_lock);
	if (!res) {
		pr_err("SLPT: error: Must have an app res struct\n");
	}

	if (!res->name) {
		pr_err("SLPT: error: Resource name must be defined\n");
		goto unlock;
	}

	if (res->type > SLPT_RES_MAX)
		res->type = SLPT_RES_MEM;

	sr = kzalloc(sizeof(*sr) + strlen(res->name) + 1, GFP_KERNEL);
	if (!sr) {
		pr_err("SLPT: error: Failed to allocate slpt resource:%s\n", res->name);
		goto unlock;
	}
	sr->res.name = (const char *)&sr[1];

	handlers = task ? &task->res_handlers : &slpt_res_handlers;

	list_for_each(pos, handlers) {
		struct slpt_res *r = list_entry(pos, struct slpt_res, link);
		ret = strcmp(r->res.name, res->name);
		if (!ret) {
			pr_err("SLPT: error: Resoure name already be registered: %s\n", res->name);
			goto error_res_name_has_been_registered;
		} else if (ret > 0) {
			break;
		}
	}
	list_add_tail(&sr->link, pos);

	sr->task = task;
	sr->res.type = res->type;
	strcpy((char *)sr->res.name, res->name);
	sr->res.addr = res->addr;
	sr->res.length = res->length;

	if (create_sys) {
		ret = slpt_create_res_sysfs_files(sr, task);
		if (ret) {
			pr_err("SLPT: error: Create res file failed:%s\n", res->name);
			goto error_res_create_sysfs_file_failed;
		}
	}

	goto unlock;
error_res_create_sysfs_file_failed:
	list_del(&sr->link);
error_res_name_has_been_registered:
	kfree(sr);
	sr = NULL;
unlock:
	mutex_unlock(&slpt_res_lock);
	return sr;
}
EXPORT_SYMBOL(slpt_register_res);

void slpt_unregister_res(struct slpt_res *sr) {
	mutex_lock(&slpt_res_lock);
	slpt_remove_res_sysfs_files(sr, sr->task);
	list_del(&sr->link);
	kfree(sr);
	mutex_unlock(&slpt_res_lock);
}
EXPORT_SYMBOL(slpt_unregister_res);

struct slpt_res *name_to_slpt_res(const char *name, struct slpt_task* task) {
	struct list_head *pos, *handlers;
	struct slpt_res *r = NULL;
	int ret;

	mutex_lock(&slpt_res_lock);

	handlers = task ? &task->res_handlers : &slpt_res_handlers;
	list_for_each(pos, handlers) {
		r = list_entry(pos, struct slpt_res, link);
		ret = strcmp(r->res.name, name);
		if (!ret) {
			goto unlock;
		} else if (ret > 0) {
			r = NULL;
			goto unlock;
		}
	}

unlock:
	mutex_unlock(&slpt_res_lock);
	return r;
}
EXPORT_SYMBOL(name_to_slpt_res);

struct slpt_app_res *slpt_app_register_res(struct slpt_app_res *res, struct slpt_task *task) {
	struct slpt_res *sr = slpt_register_res(res, res->type == SLPT_RES_FUNCTION ? 0 : 1, task);
	return sr ? &sr->res : NULL;
}
EXPORT_SYMBOL(slpt_app_register_res);

void slpt_app_unregister_res(struct slpt_app_res *res, struct slpt_task *task) {
	struct slpt_res *sr = container_of(res, struct slpt_res, res);
	slpt_unregister_res(sr);
}
EXPORT_SYMBOL(slpt_app_unregister_res);

struct slpt_app_res *name_to_slpt_app_res(const char *name, struct slpt_task *task) {
	struct slpt_res *sr = name_to_slpt_res(name, task);
	return sr ? &sr->res : NULL;
}
EXPORT_SYMBOL(name_to_slpt_app_res);

extern int jz4775_pm_enter(suspend_state_t state);

int slpt_printf(const char *fmt, ...) {
	char buf[200];
	va_list args;
	int r;

	va_start(args, fmt);
	r = vsnprintf(buf, sizeof(buf) - 1, fmt, args);
	va_end(args);

	pr_info("%s", buf);

	return 0;
}
EXPORT_SYMBOL(slpt_printf);

unsigned long slpt_app_get_api(const char *name, struct slpt_task *task) {
	unsigned long retval = 0;
	const char *str;

	if (!name) {
		pr_err("SLPT: error: name must not be NULL\n");
		return 0;
	}

	switch (name[0]) {
	case 'b':
		if (!strncmp(name, "battery-", sizeof("battery-") - 1)) {
			str = name + sizeof("battery-") - 1;
			if (!strcmp(str, "set-struct")) {
				retval = (unsigned long) slpt_app_set_battery_struct;
			} else if (!strcmp(str, "set-voltage")) {
				retval = (unsigned long) slpt_app_set_battery_voltage;
			} else if (!strcmp(str, "get-low-voltage")) {
				retval = (unsigned long) slpt_app_get_battery_low_voltage;
			}
		}
	case 'r':
		if (!strcmp(name, "res-register")) {
			retval = (unsigned long) slpt_app_register_res;
		} else if (!strcmp(name, "res-unregister")) {
			retval = (unsigned long) slpt_app_unregister_res;
		} else if (!strcmp(name, "res-get")) {
			retval = (unsigned long) name_to_slpt_app_res;
		}
		break;
	case 'p':
		if (!strcmp(name, "pm-suspend-enter")) {
			retval = (unsigned long) jz4775_pm_enter;
		} else if (!strcmp(name, "printf")) {
			retval = (unsigned long) slpt_printf;
		}
		break;
	default:
		break;
	}
	if (!retval) {
		pr_err("SLPT: error: no api named (%s)\n", name);
	}

	return retval;
}
EXPORT_SYMBOL(slpt_app_get_api);

static ssize_t firmware_data_read(struct file *filp, struct kobject *kobj,
				  struct bin_attribute *bin_attr,
				  char *buffer, loff_t offset, size_t count)
{
	struct slpt_res *sr = container_of(kobj, struct slpt_res, kobj);
	size_t length = sr->res.length;
	void *data_buf = sr->res.addr;

	if (offset >= length) {
		return 0;
	}

	if (count + offset > length) {
		count = length - offset;
	}

	if (count)
		memcpy(buffer, data_buf + offset, count);

	return count;
}

static ssize_t firmware_data_write(struct file *filp, struct kobject *kobj,
				   struct bin_attribute *bin_attr,
				   char *buffer, loff_t offset, size_t count)
{
	struct slpt_res *sr = container_of(kobj, struct slpt_res, kobj);
	size_t length = sr->res.length;
	void *data_buf = sr->res.addr;
	size_t retval = count;

	if (offset >= length) {
		return retval;
	}

	if (count + offset > length) {
		count = length - offset;
	}

	if (count)
		memcpy(data_buf + offset, buffer, count);

	return retval;
}

static struct bin_attribute slpt_res_data_bin_attr = {
	.attr = { .name = "data", .mode = 0666 },
	.size = 0,
	.read = firmware_data_read,
	.write = firmware_data_write,
};

static ssize_t length_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
	size_t count;
	struct slpt_res *sr = container_of(kobj, struct slpt_res, kobj);

	mutex_lock(&slpt_res_lock);
	count = sprintf(buf, "%d\n", sr->res.length);
	mutex_unlock(&slpt_res_lock);

	return count;
}

#define length_store NULL

slpt_attr(length);

#define slpt_res_sysfs_type(t) (t == SLPT_RES_INT || t == SLPT_RES_MEM )

static int slpt_create_res_sysfs_files(struct slpt_res *sr, struct slpt_task *task) {
	struct kobject *kobj;
	struct slpt_app_res *res = &sr->res;
	int ret;

	if (!slpt_res_sysfs_type(res->type)) {
		pr_err("SLPT: error: Invalid res type to create file , name: %s type :%d\n", res->name, res->type);
		ret = -EINVAL;
		goto error_res_sysfs_not_valid;
	}

	kobj = task ? &task->kobj_res : slpt_res_kobj;
	BUG_ON(!kobj);

	ret = kobject_init_and_add(&sr->kobj, &slpt_kobj_ktype, kobj, "%s", res->name);
	if (ret) {
		pr_err("SLPT: error: res kobj init failed: %s\n", res->name);
		ret = -ENOMEM;
		goto error_res_kobj_init_failed;
	}

	ret = sysfs_create_file(&sr->kobj, &length_attr.attr);
	if (ret) {
		pr_err("SLPT: error: res create length sysfs file failed: %s\n", res->name);
		ret = -ENOMEM;
		goto error_res_create_length_sysfs_file_failed;
	}

	ret = sysfs_create_bin_file(&sr->kobj, &slpt_res_data_bin_attr);
	if (ret) {
		pr_err("SLPT: error: res create data sysfs file failed: %s\n", res->name);
		ret = -ENOMEM;
		goto error_res_create_data_sysfs_file_failed;
	}

	return 0;
error_res_create_data_sysfs_file_failed:
	sysfs_remove_file(&sr->kobj, &length_attr.attr);
error_res_create_length_sysfs_file_failed:
	kobject_put(&sr->kobj);
error_res_kobj_init_failed:
error_res_sysfs_not_valid:
	return ret;
}

static void slpt_remove_res_sysfs_files(struct slpt_res *sr, struct slpt_task *task) {
	if (!slpt_res_sysfs_type(sr->res.type))
		return ;

	sysfs_remove_bin_file(&sr->kobj, &slpt_res_data_bin_attr);
	sysfs_remove_file(&sr->kobj, &length_attr.attr);
	kobject_put(&sr->kobj);
}
