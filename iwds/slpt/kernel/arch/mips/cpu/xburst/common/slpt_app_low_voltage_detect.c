#include <config.h>
#include <common.h>

#include <linux/err.h>
#include <asm/io.h>

#include <slpt_app.h>
#include <slpt_irq.h>

#include <slpt_app_low_voltage_detect.h>

static int save_irq_mask0;
static int save_irq_mask1;

static int is_disable_other_irq = 0;
static int low_pmu_voltage_detect_enable = 0;
static int low_pmu_voltage_min = 0;
static int low_pmu_voltage_max = 0;

static int gpio_group = -1;

struct list_head low_voltage_handler = LIST_HEAD_INIT(low_voltage_handler);

void set_low_pmu_voltage_detect_state(int min_vol, int max_vol) {
	low_pmu_voltage_min = min_vol;
	low_pmu_voltage_max = max_vol;

	if (low_pmu_voltage_min == 0 && low_pmu_voltage_max == 0)
		low_pmu_voltage_detect_enable = 0;
	else
		low_pmu_voltage_detect_enable = 1;
}

void register_low_voltage_notify(struct low_voltage_notify *no) {
	list_add_tail(&no->link, &low_voltage_handler);
}

void unregister_low_voltage_notify(struct low_voltage_notify *no) {
	list_del(&no->link);
}

void low_voltage_notify(void) {
	struct list_head *pos, *n;
	list_for_each_safe(pos, n, &low_voltage_handler) {
		struct low_voltage_notify *no = list_entry(pos, struct low_voltage_notify, link);
		no->callback(no);
	}
}

static void save_irq_mask(void)
{
	save_irq_mask0 = readl(0xB0001004);
	save_irq_mask1 = readl(0xB0001024);
}

static void restore_irq_mask(void)
{
	writel(save_irq_mask0, 0xB0001004);
	writel(save_irq_mask0, 0xB0001024);
}

static void mask_other_irq(void)
{
	int irq_mask0 = 0xffffffff;
	int irq_mask1 = 0xffffffff;

	irq_mask0 &= ~(1 << 25);	/* enable TCU2 irq */

	writel(irq_mask0, 0xB0001004);
	writel(irq_mask1, 0xB0001024);
}

static void disable_other_irq(void)
{
	save_irq_mask();
	mask_other_irq();
}

static void enable_other_irq(void)
{
	restore_irq_mask();
}

static void print_mask(void)
{
	printf("***********************\n");
	printf("irq_mask0: %x\n", readl(0xB0001004));
	printf("irq_mask1: %x\n", readl(0xB0001024));
	printf("***********************\n");
}

int low_pmu_voltage_mode(void)
{
	return is_disable_other_irq;
}

void low_pmu_voltage_detect(int voltage)
{
	if (low_pmu_voltage_detect_enable != 1)
		return ;

	if (low_pmu_voltage_min >= low_pmu_voltage_max) {
		printf("****************************\n");
		printf("**low_pmu_voltage_min = %d**\n", low_pmu_voltage_min);
		printf("**low_pmu_voltage_max = %d**\n", low_pmu_voltage_max);
		printf("****************************\n");
		return ;
	}

	if (voltage < low_pmu_voltage_min && !is_disable_other_irq) {
		disable_other_irq();
		is_disable_other_irq = 1;
		low_voltage_notify();
	} else if (voltage > low_pmu_voltage_max && is_disable_other_irq) {
		enable_other_irq();
		is_disable_other_irq = 0;
		low_voltage_notify();
	}
}

static int slpt_pmu_exit(void)
{
	/* When the PMU interrupt occurs, exit slpt back to kernel.
	  * so enable other irq.
	  */
	if (is_disable_other_irq == 1) {
		enable_other_irq();
		is_disable_other_irq = 0;
		low_voltage_notify();
	}

	return 0;
}
SLPT_ARCH_EXIT_EVERYTIME(slpt_pmu_exit);

int slpt_mask_init(void)
{
	/* if low pmu voltage detect is enable, print mask. */
	if (low_pmu_voltage_detect_enable == 1)
		print_mask();

	return 0;
}
SLPT_ARCH_INIT_EVERYTIME(slpt_mask_init);

static int slpt_pmu_init(void)
{
	int gpio_port;

	gpio_port = slpt_kernel_get_pmu_irq_gpio();
	if (gpio_port >= 0)
		gpio_group = gpio_port / 32;
	else
		gpio_group = -1;
	return 0;
}
SLPT_ARCH_INIT_ONETIME(slpt_pmu_init);
