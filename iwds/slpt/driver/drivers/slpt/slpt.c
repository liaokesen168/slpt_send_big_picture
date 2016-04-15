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

#define KB (1024)
#define MB (1024 * 1024)

#define SLPT_RESERVE_ADDR (0x80000000 + 250 * MB)

#define SLPT_LOAD_ADDR 0x8fa00000
#define SLPT_RUN_ADDR 0x8fa00000
#define SLPT_LIMIT_SIZE (6 * MB)


#define TCSM_BASE 	(0xb3422000)
#define RETURN_ADDR 	(TCSM_BASE+0)
#define REG_ADDR 	(TCSM_BASE+4)

#define reg_write(reg, value)					\
	do {										\
		(*(volatile unsigned int *)reg) = value;	\
	} while (0)

#define reg_read(reg) (*(volatile unsigned int *)reg)

#if 0
struct slpt_task {
	const char *name;
	const char *bin_desc;
	const struct firmware *task_fw;

	void *load_at_addr;
	void *run_at_addr;
	void *init_addr;
	void *exit_addr;
	size_t size;

	struct {
		unsigned int is_loaded:1;
		unsigned int is_registered:1;
	} state;

	struct kobject kobj;
	struct kobject kobj_res;
	struct attribute_group group;

	struct list_head link;		/* link of task */
	struct list_head res_handlers;	/* handlers of resource */
};
#endif

static DEFINE_MUTEX(slpt_lock);
static LIST_HEAD(slpt_handlers);

struct kset *slpt_kset;
struct kobject *slpt_kobj;
struct kobject *slpt_apps_kobj;
struct kobject *slpt_res_kobj;

/* currently runing task */
static struct slpt_task *slpt_cur = NULL;

/* currently been selected task, when suspend we run it, if enable */
static struct slpt_task *slpt_select = NULL;

#define STATUS_SIZE (100)
#define SLPT_NAME_SIZE (100)
#define SLPT_EMPTY_NAME ("")

static char slpt_select_status[STATUS_SIZE] = "none\n";
static char slpt_select_request_name[SLPT_NAME_SIZE] = SLPT_EMPTY_NAME;
static const char *slpt_select_apply_name = SLPT_EMPTY_NAME;

/* enable the selected task to be run when suspend */
static int slpt_select_enable = 0;


#define slpt_size_check(len) (((len) >= SLPT_NAME_SIZE) || (!(len)))

static int slpt_create_task_sysfs_file(struct slpt_task* task);
static void slpt_remove_task_sysfs_file(struct slpt_task *task);
static int slpt_reload_all_task(void);

static inline void slpt_set_cur(struct slpt_task *task) {
	slpt_cur = task;
}

/**
 * slpt_get_cur() - get current slpt task
 *
 * Just called by slpt*.c
 */
struct slpt_task *slpt_get_cur(void) {
	return slpt_cur;
}
EXPORT_SYMBOL(slpt_get_cur);

static inline int slpt_select_task(struct slpt_task *task) {
	if (!task) {
		slpt_select_apply_name = SLPT_EMPTY_NAME;
		slpt_select = NULL;
		slpt_select_enable = 0;
	} else {
		slpt_select_apply_name = task->name;
		slpt_select = task;
		slpt_select_enable = 0;
	}

	return 0;
}

static inline void slpt_set_select_enable(int enable) {
	if (slpt_select)
		slpt_select_enable = enable;
}

static int slpt_probe(struct platform_device *pdev) {
	return 0;
}

static int slpt_remove(struct platform_device *pdev) {
	return 0;
}

int slpt_suspend(struct platform_device *pdev, pm_message_t state) {
	return 0;
}

int slpt_resume(struct platform_device *pdev) {
	return 0;
}

static struct platform_driver slpt_driver = {
	.probe = slpt_probe,
	.remove  = slpt_remove,
	.driver.name = "slpt",
	.suspend = slpt_suspend,
	.resume = slpt_resume,
};

static struct platform_device slpt_device = {
	.name = "slpt",
};

static inline int slpt_is_loaded(struct slpt_task *task) {
	return task->state.is_loaded;
}

static inline int slpt_is_registered(struct slpt_task *task) {
	return task->state.is_registered;
}

static int slpt_is_inited(struct slpt_task *task) {
	return task->state.is_inited;
}

static int copy_pages_to_addr(void *dest, struct page **page, size_t size) {
	int i;
	void *page_data;

	for (i = 0; i < PFN_DOWN(size); ++i) {
		page_data = kmap(page[i]);
		memcpy(dest, page_data, PAGE_SIZE);
		kunmap(page[i]);
		dest += PAGE_SIZE;
	}
	if (size % PAGE_SIZE) {
		page_data = kmap(page[i]);
		memcpy(dest, page_data, size % PAGE_SIZE);
		kunmap(page[i]);
	}
	pr_info("SLPT: size: %d, pages: %d", size, i);
	return 0;
}

static int load_task(struct slpt_task *task) {
	const struct firmware *fw = task->task_fw;

	if (!fw || !fw->size || !task->load_at_addr)
		return -ENOMEM;

	if (fw->data) {
		memcpy(task->load_at_addr, fw->data, fw->size);
	} else if (fw->pages) {
		copy_pages_to_addr(task->load_at_addr, fw->pages, fw->size);
	} else {
		return -ENOMEM;
	}
	pr_info("SLPT: cp \"%s\"  %d bytes to addr %p\n", task->bin_desc, fw->size, task->load_at_addr);
	return 0;
}

static int slpt_load_task(struct slpt_task *task) {

	pr_info("SLPT: %s", __FUNCTION__);

	if (!slpt_is_registered(task)) {
		pr_err("SLPT: error: Load no registered slpt task: %s", task->name);
		return -ENODEV;
	}

	if (slpt_is_loaded(task))
		return 0;

	if (load_task(task)) {
		pr_err("SLPT: error: Failed load task fw: %s\n", task->bin_desc);
		return -ENOMEM;
	}

	return 0;
}

#define bigger_out(x, start, size) ((x) >= ((start) + (size)))
#define no_intersection(x1, s1, x2, s2) ((bigger_out(x1, x2, s2)) || (bigger_out(x2, x1, s1)))

#define in_area(x, start, size) (((x) >= (start)) && ((x) < (start) + (size)))

static inline int in_reserve_area(size_t x1, size_t s1) {
	return  in_area(x1, SLPT_RESERVE_ADDR, SLPT_LIMIT_SIZE) && in_area(x1 + s1 - 1, SLPT_RESERVE_ADDR, SLPT_LIMIT_SIZE);
}

static int slpt_get_fw_info(struct slpt_task *task) {
	unsigned int *a;

	task->size = task->task_fw->size;
	if (!(task->load_at_addr && task->run_at_addr)) {
		a = (unsigned int *)(task->task_fw->data ? task->task_fw->data : page_address(task->task_fw->pages[0]));
		a = (unsigned int *)((unsigned char *)a + 0x380);
		task->load_at_addr = (void *)a[0];
		task->run_at_addr = (void *)a[1];
		task->init_addr = (void *)a[2];
		task->exit_addr = (void *)a[3];
	}
	if (!in_area(task->run_at_addr, task->load_at_addr, task->size)) {
		return -EFAULT;
	}

	return 0;
}

static struct slpt_task *name_to_slpt_task_internal(const char *name) {
	struct list_head *pos;

	list_for_each(pos, &slpt_handlers) {
		struct slpt_task *t = list_entry(pos, struct slpt_task, link);
		pr_info("SLPT: --> %s\n", t->name);
		if (!strcmp(t->name, name))
			return t;
	}
	return NULL;
}

struct slpt_task *name_to_slpt_task(const char *name) {
	struct slpt_task *t;

	mutex_lock(&slpt_lock);
	t = name_to_slpt_task_internal(name);
	mutex_unlock(&slpt_lock);

	return t;
}
EXPORT_SYMBOL(name_to_slpt_task);

static int slpt_init_task_internal(struct slpt_task *task) {
	int (*init)(unsigned long api_addr, struct slpt_task *task);
	int ret;

	if (slpt_is_inited(task))
		return 0;

	init = (int (*)(unsigned long api_addr, struct slpt_task *task)) task->init_addr;
	if (!init) {
		pr_info("SLPT: info: task:%s no init addr\n", task->name);
		ret = -ENODEV;
		goto return_ret;
	}

	ret = init((unsigned long) slpt_app_get_api, task);
	if (ret) {
		pr_err("SLPT: error: slpt task :%s init failed\n", task->name);
		goto return_ret;
	}
	task->state.is_inited = 1;

return_ret:
	return ret;
}

int slpt_init_task(struct slpt_task *task) {
	int ret;

	mutex_lock(&slpt_lock);
	ret = slpt_init_task_internal(task);
	mutex_unlock(&slpt_lock);

	return ret;
}
EXPORT_SYMBOL(slpt_init_task);

static int slpt_exit_task_internal(struct slpt_task *task) {
	int (*exit)(unsigned long api_addr, struct slpt_task *task);

	if (!slpt_is_inited(task))
		return 0;

	exit = (int (*)(unsigned long api_addr, struct slpt_task *task)) task->exit_addr;
	if (!exit) {
		pr_info("SLPT: info: task:%s no exit addr\n", task->name);
		return -ENODEV;
	}

	task->state.is_inited = 0;

	return exit((unsigned long) slpt_app_get_api, task);
}

int slpt_exit_task(struct slpt_task *task) {
	int ret;

	mutex_lock(&slpt_lock);
	ret = slpt_exit_task_internal(task);
	mutex_unlock(&slpt_lock);

	return ret;
}
EXPORT_SYMBOL(slpt_exit_task);

/**
 * slpt_register_task() - register and slpt task
 *
 * @task->load_at_addr: address to load firmware, and if NULL, we will find it in firmware
 * @task->run_at_addr:  address to app's entrypiont, and if NULL, we will find it in firmware
 * @task->init_addr: address to app's init address
 * @task->exit_addr:  address to app's init address
 * @task->bin_desc:  descriptor of your firmware, use to get your firmware
 * @task->name:      task name, is a unique identification for the task
 *
 * @return_value: 0 if successed, nonzero if failed.
 */
int slpt_register_task(struct slpt_task *task, int init) {
	int ret = 0;
	size_t name_len;
	struct list_head *pos;
	struct slpt_task *t;

	mutex_lock(&slpt_lock);

	task->state.is_loaded = 0;
	task->state.is_registered = 0;
	task->state.is_inited = 0;

	if (!task->name) {
		ret = -EINVAL;
		pr_err("SLPT: error: task name must not be null\n");
		goto unlock;
	}

	name_len = strlen(task->name) + 1;
	if (slpt_size_check(name_len)) {
		ret = -EINVAL;
		pr_err("SLPT: error: task name len(%u) should in the range of (0 to %d)\n", name_len, SLPT_NAME_SIZE);
		goto unlock;
	}

	if (!task->bin_desc) {
		ret = -EINVAL;
		pr_err("SLPT: error: bin desc must not be null\n");
		goto unlock;
	}

	t = name_to_slpt_task_internal(task->name);
	if (t) {
		ret = -EINVAL;
		pr_err("SLPT: error: name already registered by (%s, %p, %d)\n", t->bin_desc, t->load_at_addr, t->size);
		goto unlock;
	}

	ret = request_firmware(&task->task_fw, task->bin_desc, &slpt_device.dev);
	if (ret) {
		pr_err("SLPT: error: Failed to request firmware : (%s)\n", task->bin_desc);
		ret = -ENODEV;
		goto unlock;
	}

	if (slpt_get_fw_info(task)) {
		pr_err("SLPT: error: Failed to get firmware info : (%s)\n", task->bin_desc);
		ret = -EFAULT;
		goto error_get_fw_info_failed;
	}

	if (!in_reserve_area((size_t)task->load_at_addr, task->size)) {
		pr_err("SLPT: error: Firmware is not in reserve area\n");
		pr_err("SLPT: error: (%s %p %d)\n", task->bin_desc, task->load_at_addr, task->size);
		ret = -EINVAL;
		goto error_not_in_reserve_area;
	}

	list_for_each(pos, &slpt_handlers) {
		t = list_entry(pos, struct slpt_task, link);
		if (!no_intersection(t->load_at_addr, t->size, task->load_at_addr, task->size)) {
			pr_err("SLPT: error: The bins has intersection\n");
			pr_err("SLPT: error: (%s %p %d) and (%s %p %d)\n",
				   t->bin_desc, t->load_at_addr, t->size, task->bin_desc, task->load_at_addr, task->size);
			ret = -EFAULT;
			goto error_bins_has_intersection;
		}
		if (t->load_at_addr > task->load_at_addr)
			break;
	}

	list_add_tail(&task->link, pos);
	task->state.is_registered = 1;

	INIT_LIST_HEAD(&task->res_handlers);

	if (slpt_load_task(task)) {
		pr_err("SLPT: error: Failed to load task fw\n");
		ret = -EINVAL;
		goto error_load_task_failed;
	}
	task->state.is_loaded = 1;

	ret = slpt_create_task_sysfs_file(task);
	if (ret) {
		pr_err("SLPT: error: Failed to create task sysfs file: %s\n", task->name);
		ret = -ENOMEM;
		goto error_create_task_sysfs_file_failed;
	}

	if (init) {
		ret = slpt_init_task_internal(task);
		if (ret) {
			pr_info("SLPT: info: slpt_init_task retrun with error code: (%s, %d)\n", task->name, ret);
			goto error_init_task_failed;
		}
	}

	goto unlock;
error_init_task_failed:
	slpt_remove_task_sysfs_file(task);
error_create_task_sysfs_file_failed:
	task->state.is_loaded = 0;
error_load_task_failed:
	task->state.is_registered = 0;
	list_del(&task->link);
error_bins_has_intersection:
error_not_in_reserve_area:
error_get_fw_info_failed:
	release_firmware(task->task_fw);
unlock:
	mutex_unlock(&slpt_lock);
	return ret;
}
EXPORT_SYMBOL(slpt_register_task);

void slpt_unregister_task(struct slpt_task *task) {
	mutex_lock(&slpt_lock);
	slpt_remove_task_sysfs_file(task);
	if (slpt_cur == task)
		slpt_set_cur(NULL);
	if (slpt_select == task)
		slpt_select_task(NULL);
	if (task->task_fw)
		release_firmware(task->task_fw);
	list_del(&task->link);
	slpt_exit_task_internal(task);
	task->state.is_registered = 0;
	task->state.is_loaded = 0;
	task->state.is_inited = 0;
	kobject_put(&task->kobj_res);
	kobject_put(&task->kobj);
	mutex_unlock(&slpt_lock);
}
EXPORT_SYMBOL(slpt_unregister_task);

#if 1
static int slpt_run_target_task(struct slpt_task *task) {
	void (*run)(unsigned long api_addr, struct slpt_task *task);


	pr_info("SLPT: info: %s is running---\n", task->name);
	run = (void (*)(unsigned long api_addr, struct slpt_task *task))task->run_at_addr;
	run((unsigned long)slpt_app_get_api, task);
	pr_info("SLPT: info: done\n");
	return 0;
}
#endif

/**
 * slpt_run_task() - run slpt task
 *
 * @task: task must a registered task by call slpt_register_task()
 */
int slpt_run_task(struct slpt_task *task) {
	int ret = 0;

	mutex_lock(&slpt_lock);

	if (!task) {
		pr_err("SLPT: error: slpt task can not be null\n");
		ret = -EINVAL;
		goto unlock;
	}

	ret = slpt_load_task(task);
	if (ret) {
		pr_err("SLPT: error: Failed to load task: %s\n", task->name);
		ret = -EINVAL;
		goto unlock;
	}

	slpt_set_cur(task);

	ret = slpt_run_target_task(task);
	if (ret) {
		pr_err("SLPT: error: Failed to run task: %s\n", task->name);
		ret = -EINVAL;
		goto unlock;
	}

unlock:
	mutex_unlock(&slpt_lock);
	return ret;
}
EXPORT_SYMBOL(slpt_run_task);

size_t slpt_print_task_info(char *buf, struct slpt_task *task) {
	char *p = buf;

	p += sprintf(p, "task : %s\n", task->name);
	p += sprintf(p, "fw : %s\n", task->bin_desc);
	p += sprintf(p, "size : %u\n", task->size);
	p += sprintf(p, "load at : %p\n", task->load_at_addr);
	p += sprintf(p, "run at : %p\n", task->run_at_addr);
	p += sprintf(p, "init at : %p\n", task->init_addr);
	p += sprintf(p, "exit at : %p\n", task->exit_addr);

	return p - buf;
}

int slpt_name_len_check(const char *buf, size_t count, size_t *lenp) {
	char *p;
	size_t len;

	p = memchr(buf, '\n', count);
	len = p ? p - buf : strlen(buf);
	*lenp = len;

	return  slpt_size_check(len) ? -EINVAL : 0;
}

int slpt_scanf_task_name(const char *buf, size_t count, char **name, char *status) {
	size_t len;
	char *str;

	if (slpt_name_len_check(buf, count, &len)) {
		pr_err("SLPT: error: request task name size not valid: size:%u\n", len);
		if (status)
			sprintf(status, "%s\n", "size_invalid");
		return -1;
	}

	str = *name ? *name : kmalloc(len + 1, GFP_KERNEL);
	if (!str) {
		pr_err("SLPT: error: Allocate slpt name failed\n");
		if (status)
			sprintf(status, "%s\n", "no_mem");
		return -2;
	}

	memcpy(str, buf, len);
	str[len] = '\0';
	*name = str;
	return 0;
}

static int slpt_reload_all_task(void) {
	struct list_head *pos;

	mutex_lock(&slpt_lock);
	list_for_each(pos, &slpt_handlers) {
		struct slpt_task *t = list_entry(pos, struct slpt_task, link);
		pr_info("SLPT: --> %s\n", t->name);
		load_task(t);
	}
	mutex_unlock(&slpt_lock);
	return 0;
}

static ssize_t reload_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
	return 0;
}

static ssize_t reload_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
	struct slpt_task *task = container_of(kobj, struct slpt_task, kobj);

	mutex_lock(&slpt_lock);
	load_task(task);
	mutex_unlock(&slpt_lock);
	return count;
}

slpt_attr(reload);

static ssize_t info_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
	struct slpt_task *task = container_of(kobj, struct slpt_task, kobj);

	return slpt_print_task_info(buf, task);
}

static ssize_t info_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
	struct slpt_task *task = container_of(kobj, struct slpt_task, kobj);

	pr_info("SLPT: info: task name :%s\n", task->name);
	return count;
}

slpt_attr(info);

static char slpt_new_res_status[STATUS_SIZE] = "none\n";
#define RES_LENGHT 200

static ssize_t add_res_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {

	return sprintf(buf, "%s", slpt_new_res_status);
}

static ssize_t add_res_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
	struct slpt_task *task = container_of(kobj, struct slpt_task, kobj);
	char name[SLPT_NAME_SIZE];
	char *p = name;
	struct slpt_app_res *res;
	struct slpt_res *sr;
	size_t len;

	pr_info("SLPT: info: test: (%s) count:%u strlenbuf:%u\n", buf, count, strlen(buf));

	mutex_lock(&slpt_lock);

	if (slpt_scanf_task_name(buf, count, &p, slpt_new_res_status)) {
		pr_err("SLPT: error: Failed to get task name from buf\n");
		goto return_count;
	}

	len = strlen(name) + 1;
	res = kzalloc(sizeof(*res) + len + RES_LENGHT, GFP_KERNEL);
	if (!res) {
		pr_err("SLPT: error: Allocate task memory failed\n");
		sprintf(slpt_new_res_status, "%s\n", "no_mem");
		goto return_count;
	}
	res->name = p = (char *)&res[1];
	memcpy(p, name, len);

	res->length = RES_LENGHT;
	res->type = SLPT_RES_MEM;
	res->addr = (void *)res + sizeof(*res) + len;

	sr = slpt_register_res(res, 1, task);
	if (!sr) {
		pr_err("SLPT: error: register res failed\n");
		sprintf(slpt_new_res_status, "%s\n", "failed");
		goto error_register_res_failed;
	}

	sprintf(slpt_new_res_status, "%s\n", "success");

	goto return_count;
error_register_res_failed:
	kfree(res);
return_count:
	mutex_unlock(&slpt_lock);
	return count;
}

slpt_attr(add_res);

static char slpt_rm_res_status[STATUS_SIZE] = "none\n";

static ssize_t rm_res_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
	size_t count;

	mutex_lock(&slpt_lock);
	count = sprintf(buf, "%s", slpt_rm_res_status);
	mutex_unlock(&slpt_lock);

	return count;
}

static ssize_t rm_res_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
	char name[SLPT_NAME_SIZE];
	char *p = name;
	struct slpt_res *sr;
	struct slpt_task *task = container_of(kobj, struct slpt_task, kobj);

	if (slpt_scanf_task_name(buf, count, &p, slpt_rm_res_status)) {
		pr_err("SLPT: error: Failed to get task name from buf\n");
		goto return_count;
	}

	sr = name_to_slpt_res(name, task);
	if (!sr) {
		pr_info("SLPT: error: Failed to get slpt res:%s\n", name);
		sprintf(slpt_rm_res_status, "%s\n", "no_res");
		goto return_count;
	}

	slpt_unregister_res(sr);
	kfree(sr);
	sprintf(slpt_rm_res_status, "%s\n", "success");

	goto return_count;
return_count:
	return count;
}

slpt_attr(rm_res);

#define exit_show NULL

static ssize_t exit_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
	struct slpt_task *task = container_of(kobj, struct slpt_task, kobj);

	slpt_exit_task(task);

	return count;
}

slpt_attr(exit);

#define init_show NULL

static ssize_t init_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
	struct slpt_task *task = container_of(kobj, struct slpt_task, kobj);

	slpt_init_task(task);

	return count;
}

slpt_attr(init);

static struct attribute *slpt_app_g[] = {
	&info_attr.attr,
	&add_res_attr.attr,
	&rm_res_attr.attr,
	&init_attr.attr,
	&exit_attr.attr,
	&reload_attr.attr,
	NULL,
};

static void slpt_kobj_release(struct kobject *kobj) {
	pr_debug("kobject: (%p): %s\n", kobj, __func__);
}

struct kobj_type slpt_kobj_ktype = {
	.release = slpt_kobj_release,
	.sysfs_ops = &kobj_sysfs_ops,
};

static int slpt_create_task_sysfs_file(struct slpt_task* task) {
	int ret;

	ret = kobject_init_and_add(&task->kobj, &slpt_kobj_ktype, slpt_apps_kobj, "%s", task->name);
	if (ret) {
		pr_err("SLPT: error: Failed to add slpt task kobj: %s\n", task->name);
		ret =  -ENOMEM;
		goto error_task_kobj_create_failed;
	}

	ret = kobject_init_and_add(&task->kobj_res, &slpt_kobj_ktype, &task->kobj, "%s", "res");
	if (ret) {
		pr_err("SLPT: error: Failed to add slpt task kobj: %s\n", task->name);
		ret = -ENOMEM;
		goto error_task_res_kobj_create_failed;
	}

	task->group.name = NULL;
	task->group.attrs = slpt_app_g;

	ret = sysfs_create_group(&task->kobj, &task->group);
	if (ret) {
		pr_err("SLPT: error: slpt apps kobject create failed\n");
		ret = -ENOMEM;
		goto error_task_sys_group_create_failed;
	}

	return 0;
error_task_sys_group_create_failed:
	kobject_put(&task->kobj_res);
error_task_res_kobj_create_failed:
	kobject_put(&task->kobj);
error_task_kobj_create_failed:
	return ret;
}

static void slpt_remove_task_sysfs_file(struct slpt_task *task) {
	sysfs_remove_group(&task->kobj, &task->group);
	kobject_put(&task->kobj);
}

static struct slpt_task test_task;
static unsigned int save_tcsm[2] = {0, 1};
extern int jz4775_pm_enter(suspend_state_t state);

void slpt_run_test_task(struct slpt_task *task) {
	unsigned int temp[2];

	save_tcsm[0] = reg_read(RETURN_ADDR);
	save_tcsm[1] = reg_read(REG_ADDR);
	reg_write(RETURN_ADDR, (unsigned int)jz4775_pm_enter);
	reg_write(REG_ADDR, 4);
	slpt_run_task(task);

	temp[0] = reg_read(RETURN_ADDR);
	temp[1] = reg_read(REG_ADDR);
	reg_write(RETURN_ADDR, save_tcsm[0]);
	reg_write(REG_ADDR, save_tcsm[1]);
	save_tcsm[0] = temp[0];
	save_tcsm[1] = temp[1];
}

static char slpt_run_status[STATUS_SIZE] = "none\n";

static ssize_t run_task_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
	pr_info("SLPT: info: tcsm[0]:0x%x tcsm[1]:0x%x\n", save_tcsm[0], save_tcsm[1]);

	return 0;
}

static DEFINE_SPINLOCK(slpt_spin_lock);

static ssize_t run_task_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
	unsigned long flags;
	struct slpt_task *task;
	char name[SLPT_NAME_SIZE];
	char *p = name;

	pr_info("p:%p\n", p);
	if (slpt_scanf_task_name(buf, count, &p, slpt_run_status)) {
		pr_err("SLPT: error: Failed to get task name from buf\n");
		goto return_count;
	}

	task = name_to_slpt_task(name);
	if (!task) {
		pr_err("SLPT: error: No task named (%s)\n", name);
		sprintf(slpt_run_status, "%s\n", "no_task");
		goto return_count;
	}

	spin_lock_irqsave(&slpt_spin_lock, flags);
	slpt_run_task(task);
	spin_unlock_irqrestore(&slpt_spin_lock, flags);

return_count:
	return count;
}

slpt_attr(run_task);

static ssize_t task_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
	size_t count;

	mutex_lock(&slpt_lock);
	count = sprintf(buf, "%s", slpt_select_status);
	mutex_unlock(&slpt_lock);

	return count;
}

static ssize_t task_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
	struct slpt_task *task;
	char *p = slpt_select_request_name;
	pr_info("SLPT: info: test: (%s) count:%u strlenbuf:%u\n", buf, count, strlen(buf));

	mutex_lock(&slpt_lock);

	pr_info("p:%p\n", p);
	if (slpt_scanf_task_name(buf, count, &p, slpt_select_status)) {
		pr_err("SLPT: error: Failed to get task name from buf\n");
		goto unlock;
	}

	task = name_to_slpt_task_internal(slpt_select_request_name);
	if (!task) {
		pr_err("SLPT: error: No task name is (%s) be found\n", slpt_select_request_name);
		sprintf(slpt_select_status, "%s\n", "no_task");
		goto unlock;
	} else {
		/* task be found */
		sprintf(slpt_select_status, "%s\n", "success");
		slpt_select_task(task);
	}

unlock:
	mutex_unlock(&slpt_lock);
	return count;
}

slpt_attr(task);

static ssize_t apply_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
	size_t count;

	mutex_lock(&slpt_lock);
	count = sprintf(buf, "%s\n", slpt_select_apply_name);
	mutex_unlock(&slpt_lock);

	return count;
}

#define apply_store NULL

slpt_attr(apply);

static ssize_t enable_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
	size_t count;

	mutex_lock(&slpt_lock);
	count = sprintf(buf, "%d\n", slpt_select_enable);
	mutex_unlock(&slpt_lock);

	return count;
}

static ssize_t enable_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
	int enable = simple_strtol(buf, NULL, 10);

	mutex_lock(&slpt_lock);
	if (enable == 0 || enable == 1) {
		slpt_set_select_enable(enable);
	}
	mutex_unlock(&slpt_lock);

	return count;
}

slpt_attr(enable);

static char slpt_add_status[STATUS_SIZE] = "none\n";

static ssize_t add_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
	size_t count;

	mutex_lock(&slpt_lock);
	count = sprintf(buf, "%s", slpt_add_status);
	mutex_unlock(&slpt_lock);

	return count;
}

static void fix_name(char *str) {
	while (*str != '\0') {
		if (*str == '.' || *str == '\\')
			*str = '-';
		++str;
	}
}

static ssize_t add_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
	char name[SLPT_NAME_SIZE];
	char *p = name;
	struct slpt_task *task;
	size_t len;
	int ret;

	if (slpt_scanf_task_name(buf, count, &p, slpt_add_status)) {
		pr_err("SLPT: error: Failed to get task name from buf\n");
		goto return_count;
	}

	len = strlen(name) + 1;
	task = kzalloc(sizeof(*task) + len * 2, GFP_KERNEL);
	if (!task) {
		pr_err("SLPT: error: Allocate task memory failed\n");
		sprintf(slpt_add_status, "%s\n", "no_mem");
		goto return_count;
	}

	task->bin_desc = p = (char *)&task[1];
	memcpy(p, name, len);
	task->name = p = (char *)&task[1] + len;
	fix_name(name);
	memcpy(p, name, len);

	ret = slpt_register_task(task, 1);
	if (ret) {
		pr_info("SLPT: error: Failed to register slpt task:%s\n", name);
		sprintf(slpt_add_status, "%s\n", "failed");
		goto error_slpt_register_task_failed;
	}
	sprintf(slpt_add_status, "%s\n", "success");

	goto return_count;
error_slpt_register_task_failed:
	kfree(task);
return_count:
	return count;
}

slpt_attr(add);

static char slpt_remove_status[STATUS_SIZE] = "none\n";

static ssize_t remove_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
	size_t count;

	mutex_lock(&slpt_lock);
	count = sprintf(buf, "%s", slpt_remove_status);
	mutex_unlock(&slpt_lock);

	return count;
}

static ssize_t remove_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
	char name[SLPT_NAME_SIZE];
	char *p = name;
	struct slpt_task *task;

	if (slpt_scanf_task_name(buf, count, &p, slpt_remove_status)) {
		pr_err("SLPT: error: Failed to get task name from buf\n");
		goto return_count;
	}

	task = name_to_slpt_task(name);
	if (!task) {
		pr_info("SLPT: error: Failed to get slpt task:%s\n", name);
		sprintf(slpt_remove_status, "%s\n", "no_task");
		goto return_count;
	}

	slpt_unregister_task(task);
	kfree(task);
	sprintf(slpt_remove_status, "%s\n", "success");

	goto return_count;
return_count:
	return count;
}

slpt_attr(remove);

ssize_t task_info_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
	char *p = buf;
	struct list_head *pos;

	list_for_each(pos, &slpt_handlers) {
		struct slpt_task *task = list_entry(pos, struct slpt_task, link);

		p += slpt_print_task_info(p, task);
		p += sprintf(p, "\n");
	}

	return p - buf;
}

ssize_t task_info_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {

	return count;
}

slpt_attr(task_info);

static struct attribute *test_g[] = {
	&run_task_attr.attr,
	&task_attr.attr,
	&apply_attr.attr,
	&enable_attr.attr,
	&add_attr.attr,
	&remove_attr.attr,
	&task_info_attr.attr,
	NULL,
};

static struct attribute_group slpt_attrs_g = {
	.attrs = test_g,
	.name = NULL,
};

static char data_buf[] = "it is a test data buf\n";
#define DATA_BUF_SIZE (sizeof(data_buf))

static ssize_t firmware_data_read(struct file *filp, struct kobject *kobj,
				  struct bin_attribute *bin_attr,
				  char *buffer, loff_t offset, size_t count)
{
	if (offset >= DATA_BUF_SIZE) {
		return 0;
	}

	if (count + offset > DATA_BUF_SIZE) {
		count = DATA_BUF_SIZE - offset;
	}

	if (count)
		memcpy(buffer, data_buf + offset, count);

	return count;
}

static ssize_t firmware_data_write(struct file *filp, struct kobject *kobj,
				   struct bin_attribute *bin_attr,
				   char *buffer, loff_t offset, size_t count)
{
	size_t retval = count;

	if (offset >= DATA_BUF_SIZE) {
		return retval;
	}

	if (count + offset > DATA_BUF_SIZE) {
		count = DATA_BUF_SIZE - offset;
	}

	if (count)
		memcpy(data_buf + offset, buffer, count);

	return retval;
}

static struct bin_attribute test_bin_attr = {
	.attr = { .name = "test-bin", .mode = 0644 },
	.size = 0,
	.read = firmware_data_read,
	.write = firmware_data_write,
};

static int __init slpt_init(void) {
	int ret;

	ret = platform_driver_register(&slpt_driver);
	if (ret) {
		pr_err("SLPT: error: slpt driver register failed\n");
		ret = -EINVAL;
		goto error_platform_driver_register_failed;
	}

	ret = platform_device_register(&slpt_device);
	if (ret) {
		pr_err("SLPT: error: slpt device register failed\n");
		ret = -EINVAL;
		goto error_platform_device_register_failed;
	}

	slpt_kset = kset_create_and_add("slpt", NULL, NULL);
	if (!slpt_kset) {
		pr_err("SLPT: error: slpt kset create failed\n");
		ret = -ENOMEM;
		goto error_slpt_kset_create_failed;
	}
	slpt_kobj = &slpt_kset->kobj;
	slpt_kobj->kset = slpt_kset;

	slpt_apps_kobj = kobject_create_and_add("apps", slpt_kobj);
	if (!slpt_apps_kobj) {
		pr_err("SLPT: error: slpt apps kobject create failed\n");
		ret = -ENOMEM;
		goto error_slpt_apps_kobj_create_failed;
	}

	slpt_res_kobj = kobject_create_and_add("res", slpt_kobj);
	if (!slpt_res_kobj) {
		pr_err("SLPT: error: slpt res kobject create failed\n");
		ret = -ENOMEM;
		goto error_slpt_res_kobj_create_failed;
	}

	ret = sysfs_create_group(slpt_kobj, &slpt_attrs_g);
	if (ret) {
		pr_err("SLPT: error: slpt sysfs group create failed\n");
		ret = -ENOMEM;
		goto error_create_slpt_g_failed;
	}

	ret = sysfs_create_bin_file(slpt_kobj, &test_bin_attr);
	if (ret) {
		pr_err("SLPT: error: slpt test bin sysfs create failed\n");
		ret = -ENOMEM;
		goto error_create_slpt_test_bin_failed;
	}

	return 0;
error_create_slpt_test_bin_failed:
	sysfs_remove_group(slpt_kobj, &slpt_attrs_g);
error_create_slpt_g_failed:
	kobject_put(slpt_res_kobj);
error_slpt_res_kobj_create_failed:
	kobject_put(slpt_apps_kobj);
error_slpt_apps_kobj_create_failed:
	kset_unregister(slpt_kset);
error_slpt_kset_create_failed:
	platform_device_unregister(&slpt_device);
error_platform_device_register_failed:
	platform_driver_unregister(&slpt_driver);
error_platform_driver_register_failed:
	return ret;
}

static void __exit slpt_exit(void) {
	sysfs_remove_group(slpt_kobj, &slpt_attrs_g);
	kobject_put(slpt_apps_kobj);
	kobject_put(slpt_kobj);
	platform_driver_unregister(&slpt_driver);
	platform_device_unregister(&slpt_device);
}

core_initcall_sync(slpt_init);
module_exit(slpt_exit);

static int test_slpt_init(void) {
	int ret;
#if 0
	memset(&test_task, 0, sizeof(test_task));
	test_task.name = "test-call";
	test_task.bin_desc = "slpt_firmware/slpt.bin.ihex";
	test_task.load_at_addr = 0;
	test_task.run_at_addr = 0;
	ret = slpt_register_task(&test_task, 1);
	if (ret) {
		pr_err("TEST: Failed to register slpt task\n");
		return 0;
	}

	slpt_select_task(&test_task);
	slpt_set_select_enable(1);
#endif
#if 0
	ret = slpt_run_test_task(slpt_select);
	if (ret) {
		pr_err("TEST: Failed to run slpt task\n");
		slpt_unregister_task(&test_task);
		return 0;
	}

	slpt_unregister_task(&test_task);
#endif

	return 0;
}

static void __exit test_slpt_exit(void) {
#if 0
	slpt_unregister_task(&test_task);
#endif
}

late_initcall(test_slpt_init);
module_exit(test_slpt_exit);

int jz4775_test_pm_enter(suspend_state_t state) {
	if (slpt_select_enable) {
		slpt_run_task(slpt_select);
	} else {
		pr_info("SLPT: info: no task selected, enter sleep\n");
		jz4775_pm_enter(state);
	}

	return 0;
}
