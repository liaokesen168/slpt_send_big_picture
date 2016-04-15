#include <config.h>
#include <asm/errno.h>
#include <common.h>
#include <asm/io.h>
#include <slpt.h>
#include <jzfb.h>
#include <malloc.h>
#include <linux/pr_info.h>
#include <linux/lcd_mcu.h>
#include <fb_struct.h>
#include <asm/arch/cpm.h>
#include <linux/err.h>
#include <linux/lcd_mcu.h>
#ifdef CONFIG_M200
#include <asm/r4kcache.h>
#endif

#include "./jz_mipi_dsi/jz_mipi_dsih_hal.h"
#include "./jz_mipi_dsi/jz_mipi_dsi_regs.h"
#include "./jz_mipi_dsi/jz_mipi_dsi_lowlevel.h"

#include <asm/arch/cpm.h>

#include "regs.h"

#define CONFIG_JZFB_BUFFER_ALIGN 4  /*  */
#define CONFIG_JZFB_BUFFER_NUMS  2  /* allocate buffers nums */
#ifdef CONFIG_M200
#define CONFIG_JZFB_PIXEL_ALIGN  4 /* m200 pixel align fix to 4 */
#else
#define CONFIG_JZFB_PIXEL_ALIGN  16 /* as kernel doing, and jzfb lcdc needs word align for each lines */
#endif
#define CONFIG_JZFB_GPIO_FLUSH_LCD 0 /* gpio to flush lcd or not */

#undef outl
#undef inl
#define inl(addr) (*(volatile unsigned int *)(addr))
#define outl(value, addr) (*(volatile unsigned int *)(addr) = (value))

static int jzfb_pixel_align = CONFIG_JZFB_PIXEL_ALIGN; /*default*/

static inline unsigned int reg_read(struct jzfb *jzfb, unsigned int offset) {
	return inl(jzfb->base + offset);
}

static inline void reg_write(struct jzfb *jzfb, unsigned int offset, unsigned int value) {
	outl(value, jzfb->base + offset);
}

static inline int jzfb_lcdc_is_enabled(struct jzfb *jzfb) {
	return reg_read(jzfb, LCDC_CTRL) & LCDC_CTRL_ENA;
}

static inline unsigned int bpp_to_bits(unsigned int bpp) {
	unsigned int bits;

	switch (bpp) {
	case 18: case 24: bits = 32; break;
	case 16: case 15: bits = 16; break;
	default:
		pr_info("jzfb: info: No support this bpp :%d\n", bpp);
		bits = bpp; break;
	}
	return bits;
}

static inline unsigned int jzfb_pixels_per_line(unsigned int xres) {
	return ALIGN(xres, jzfb_pixel_align);
}

static inline size_t jzfb_video_size(struct fb_videomode *mode, unsigned int bpp) {
	unsigned int line_len;
	size_t size;

	line_len = jzfb_pixels_per_line(mode->xres) * bpp_to_bits(bpp) / 8;
	size = ALIGN(line_len * mode->yres, CONFIG_JZFB_BUFFER_ALIGN);

	return size;
}

void print_fbs(struct fb_struct *fbs) {
	pr_info("fbs: xres: %u\n", fbs->xres);
	pr_info("fbs: yres: %u\n", fbs->yres);
	pr_info("fbs: bits_per_pixel: %u\n", fbs->bits_per_pixel);
	pr_info("fbs: line_len: %u\n", fbs->pixels_per_line);
	pr_info("fbs: base: 0x%p\n", fbs->base);
	pr_info("fbs: base_phys: 0x%lx\n", fbs->base_phys);
	pr_info("fbs: size: %u\n", fbs->size);
	pr_info("fbs: nums: %u\n", fbs->nums);
}

static int jzfb_alloc_mem(struct jzfb *jzfb) {
	size_t buffer_size;
	unsigned int bpp = jzfb->pdata->bpp;
	void *base;
	struct fb_struct *fbs = &jzfb->fbs;

	if (!jzfb->mode) {
		pr_err("jzfb: error: video mode can not be NULL\n");
		return -ENODEV;
	}

	buffer_size = jzfb_video_size(jzfb->mode, bpp);

	pr_info("jzfb: info: buffer size:%d\n", buffer_size);
	base = memalign(CONFIG_SYS_CACHELINE_SIZE, buffer_size * CONFIG_JZFB_BUFFER_NUMS);
	if (!base) {
		pr_err("jzfb: error: allocate buffer memroy failed\n");
		return -ENOMEM;
	}

	fbs->base = base;
#if defined(CONFIG_M200)
	fbs->base_phys = virt_to_phys(slpt_kernel_get_reserve_mem() + (base - CONFIG_SYS_SDRAM_BASE));
#else
	fbs->base_phys = virt_to_phys(base);
#endif

	pr_err("fbs: base: 0x%p\n", fbs->base);
	pr_err("fbs: base_phys: 0x%lx\n", fbs->base_phys);
	pr_err("fbs: reserve: 0x%p\n", slpt_kernel_get_reserve_mem());

	fbs->size = buffer_size;
	fbs->nums = CONFIG_JZFB_BUFFER_NUMS;

	fbs->xres = jzfb->mode->xres;
	fbs->yres = jzfb->mode->yres;
	fbs->bits_per_pixel = bpp_to_bits(jzfb->pdata->bpp);
	fbs->pixels_per_line = jzfb_pixels_per_line(jzfb->mode->xres);

	slpt_kernel_printf("reserve: %p\n", slpt_kernel_get_reserve_mem());
	slpt_kernel_printf("phys: %x\n", fbs->base_phys);

	pr_info("jzfb: allocate %u bytes start of %p/%lx\n", fbs->size, fbs->base, fbs->base_phys);

	print_fbs(fbs);

	return 0;
}

static int jzfb_supported_lcd_type(unsigned int type) {
	switch (type) {
	case LCD_TYPE_LCM:
		return 1;
	default:
		pr_err("jzfb: error: currently not support this lcd type: 0x%x\n", type);
		return 0;
	}
}

void print_framedesc(struct jzfb_framedesc *fbd) {
	pr_info("fbd: %x\n", (unsigned int) fbd);
	pr_info("fbd->next: %x\n", fbd->next);
	pr_info("fbd->databuf: %x\n", fbd->databuf);
	pr_info("fbd->id: %x\n", fbd->id);
	pr_info("fbd->cmd: %x\n", fbd->cmd);
	pr_info("fbd->offsize: %x\n", fbd->offsize);
	pr_info("fbd->page_width: %x\n",fbd->page_width);
	pr_info("fbd->cpos: %x\n", fbd->cpos);
	pr_info("fbd->desc_size: %x\n", fbd->desc_size);
}
#ifndef CONFIG_M200
int jzfb_clk_enable(struct jzfb *jzfb) {
	int ret;

	jzfb->clk = clk_get("lcd");
	if (IS_ERR(jzfb->clk)) {
		pr_err("jzfb: error: faild to get lcd clk\n");
		return -ENODEV;
	}

	ret = clk_enable(jzfb->clk);
	if (ret) {
		pr_err("jzfb: error: faild to enable lcd clk\n");
		return ret;
	}

	jzfb->s.is_clk_enabled = 1;

	return 0;
}

int jzfb_clk_disable(struct jzfb *jzfb) {
	int ret;

	jzfb->clk = clk_get("lcd");
	if (IS_ERR(jzfb->clk)) {
		pr_err("jzfb: error: faild to get lcd clk\n");
		return -ENODEV;
	}

	ret = clk_disable(jzfb->clk);
	if (ret) {
		pr_err("jzfb: error: faild to enable lcd clk\n");
		return ret;
	}

	jzfb->s.is_clk_enabled = 0;

	return 0;
}
#else

extern void disable_lcd_clk(void);
extern void enable_lcd_clk(void);

int jzfb_clk_enable(struct jzfb *jzfb) {
	enable_lcd_clk();
	jzfb->s.is_clk_enabled = 1;

	return 0;
}

int jzfb_clk_disable(struct jzfb *jzfb) {
	disable_lcd_clk();
	jzfb->s.is_clk_enabled = 0;

	return 0;
}
#endif

static int jzfb_set_par(struct jzfb *jzfb) {

	jzfb->s.is_panel_setted = 1;

	return 0;
}

static int jzfb_restore_par(struct jzfb *jzfb) {

	jzfb->s.is_panel_setted = 0;

	return 0;
}

static inline void jzfb_lcdc_enable(struct jzfb *jzfb) {
	unsigned int ctrl;

	reg_write(jzfb, LCDC_STATE, 0);
	reg_write(jzfb, LCDC_OSDS, 0);
	ctrl = reg_read(jzfb, LCDC_CTRL);
	ctrl |= LCDC_CTRL_ENA;
	ctrl &= ~LCDC_CTRL_DIS;
	reg_write(jzfb, LCDC_CTRL, ctrl);
}

static inline void jzfb_lcdc_init_gpio(struct jzfb *jzfb) {
	if (jzfb->s.is_gpio_transfer)
		slcd_data_as_output(jzfb->slcd_mode);
	else if (jzfb->pdata->lcd_type == LCD_TYPE_LCM && !jzfb->pdata->mipi_dsi)
		slcd_pin_as_function(jzfb->slcd_mode);

	pr_debug("jzfb: gpio init to %s\n", jzfb->s.is_gpio_transfer ? "gpio" : "func");

	jzfb->s.need_to_init_gpio = 0;
}

static inline void jzfb_lcdc_restore_gpio(struct jzfb *jzfb) {

	if (jzfb->s.is_gpio_transfer) {
		slcd_pin_as_function(jzfb->slcd_mode);
	}
	jzfb->s.need_to_init_gpio = 1;
}

static inline void jzfb_enable_internal(struct jzfb* jzfb) {
#if 1
	jzfb_lcdc_enable(jzfb);
#endif

	jzfb->s.is_enabled = 1;
}

static inline volatile void *jzfb_lcdc_get_framedesc(struct jzfb *jzfb) {
	volatile struct jzfb_framedesc *fbd;

	fbd = (void *)(0xA0000000 | reg_read(jzfb, LCDC_DA0));

	if (fbd->cmd & LCDC_CMD_CMD) {
		fbd = (void *)(0xA0000000 | fbd->next);
	}

	return fbd;
}

static inline void jzfb_set_base(struct jzfb *jzfb, u32 base) {
	volatile struct jzfb_framedesc *fbd;

	fbd = jzfb_lcdc_get_framedesc(jzfb);
	fbd->databuf = base;
}

static inline void jzfb_save_base(struct jzfb *jzfb) {
	volatile struct jzfb_framedesc *fbd;

	if (!jzfb->s.need_to_save_base)
		return;

	fbd = jzfb_lcdc_get_framedesc(jzfb);
	jzfb->save.base = (unsigned long)fbd->databuf;

	jzfb->s.need_to_save_base = 0;
}

static inline void jzfb_restore_base(struct jzfb *jzfb) {
	volatile struct jzfb_framedesc *fbd;

	if (jzfb->s.need_to_save_base)
		return;

	fbd = jzfb_lcdc_get_framedesc(jzfb);
	fbd->databuf = (u32)jzfb->save.base;

	jzfb->s.need_to_save_base = 1;
}

int jzfb_enable(struct jzfb *jzfb) {
	int ret = 0;

	if (!jzfb->s.is_enabled) {
		if (!jzfb->s.is_clk_enabled)
			jzfb_clk_enable(jzfb);
		if (jzfb->s.need_to_init_gpio)
			jzfb_lcdc_init_gpio(jzfb);
		if (!jzfb->s.is_panel_setted) {
			ret = jzfb_set_par(jzfb);
				if (ret) {
					pr_err("jzfb: error: Failed to set panel\n");
					goto return_ret;
			}
		}
		jzfb_enable_internal(jzfb);

		if(jzfb->pdata->mipi_dsi) {  /* if it is mipi...*/
			jz_dsih_dphy_ulpm_exit(&jzfb->dsi);
		}
	}

return_ret:
	return ret;
}

static inline void jzfb_lcdc_normal_disable(struct jzfb *jzfb) {
	unsigned int ctrl;
	int count = 1000;

	ctrl = reg_read(jzfb, LCDC_CTRL);
	ctrl |= LCDC_CTRL_DIS;
	reg_write(jzfb, LCDC_CTRL, ctrl);
	while (!(reg_read(jzfb, LCDC_STATE) & LCDC_STATE_LDD)
		   && count--) {
		udelay(10);
	}
	if (count >= 0) {
		ctrl = reg_read(jzfb, LCDC_STATE);
		ctrl &= ~LCDC_STATE_LDD;
		reg_write(jzfb, LCDC_STATE, ctrl);
	} else {
		pr_err("jzfb: error: lcdc disable state wrong");
	}
}

static inline void jzfb_lcdc_quick_disable(struct jzfb *jzfb) {
	unsigned int ctrl;

	ctrl = reg_read(jzfb, LCDC_CTRL);
	ctrl &= ~LCDC_CTRL_ENA;
	reg_write(jzfb, LCDC_CTRL, ctrl);
#ifdef CONFIG_SLCDC_DMA_CONTNUALLY_TRANSFER
	ctrl = reg_read(jzfb, SLCDC_CTRL);
	ctrl &= ~SLCDC_CTRL_DMA_EN;
	reg_write(jzfb, SLCDC_CTRL, ctrl);
#endif
}

static inline void jzfb_disable_internal(struct jzfb *jzfb) {

	if (jzfb->pdata->lcd_type != LCD_TYPE_LCM)
		jzfb_lcdc_normal_disable(jzfb);
	else
		jzfb_lcdc_quick_disable(jzfb);
}

int jzfb_disable(struct jzfb *jzfb) {

	if (jzfb->s.is_enabled) {
		if(jzfb->pdata->mipi_dsi) {/* if it is mipi...*/
			jz_dsih_dphy_ulpm_enter(&jzfb->dsi);
		}
		jzfb_disable_internal(jzfb);
		jzfb_restore_par(jzfb);
		jzfb_lcdc_restore_gpio(jzfb);
		jzfb_clk_disable(jzfb);
	}

	jzfb->s.is_enabled = 0;

	return 0;
}

static int jzfb_gpio_flush_lcd(struct jzfb *jzfb, void *base) {
	struct fb_struct *fbs = &jzfb->fbs;
	struct slcd_mode *slcd  = jzfb->slcd_mode;
	unsigned int bus_width = jzfb->pdata->smart_config.bus_width;
	unsigned int bpp = jzfb->pdata->bpp;
	unsigned int data_width = jzfb->pdata->smart_config.data_width2;
	int ret = -ENODEV;

	if (bus_width == 8) {
		ret = 0;

		if (data_width == SMART_LCD_DWIDTH_8_BIT_TWICE_TIME_PARALLEL) {
			switch (bpp) {
			case 24: slcd_write_buffer_24_twice(slcd, fbs, base); break;
			case 18: slcd_write_buffer_18_twice(slcd, fbs, base); break;
			case 16: slcd_write_buffer_16_twice(slcd, fbs, base); break;
			case 15: slcd_write_buffer_16_twice(slcd, fbs, base); break;
			default:
				ret = -ENODEV;
				break;
			}
		} else if (data_width == SMART_LCD_DWIDTH_8_BIT_THIRD_TIME_PARALLEL) {
			switch (bpp) {
			case 24: slcd_write_buffer_24_thrice(slcd, fbs, base); break;
			case 18: slcd_write_buffer_18_thrice(slcd, fbs, base); break;
			default:
				ret = -ENODEV;
				break;
			}
		} else {
			ret = -ENODEV;
		}
	}

	if (ret) {
		pr_err("jzfb: error: currently ops, buswidth:%u bpp:%d \n", bus_width, bpp);
	}

	return ret;
}

static inline void jzfb_lcdc_start_slcd_dma(struct jzfb *jzfb) {
	unsigned int reg;

#ifndef CONFIG_SLCDC_DMA_CONTNUALLY_TRANSFER
	reg = reg_read(jzfb, SLCDC_CTRL);
	reg |= SLCDC_CTRL_DMA_START;
	reg_write(jzfb, SLCDC_CTRL, reg);
#else
	reg = reg_read(jzfb, SLCDC_CTRL);
	reg &= ~SLCDC_CTRL_DMA_MODE;
	reg |= SLCDC_CTRL_DMA_EN;
	reg_write(jzfb, SLCDC_CTRL, reg);
#endif
}

static inline void jzfb_lcdc_clear_state(struct jzfb *jzfb) {
	reg_write(jzfb, LCDC_STATE, 0);
	reg_write(jzfb, LCDC_OSDS, 0);
	/* reg_write(jzfb, SLCDC_STATE, 0); */
}

static inline void jzfb_lcdc_enable_frame_end_irq(struct jzfb *jzfb) {
	unsigned int reg = reg_read(jzfb, LCDC_CTRL);
	reg |= LCDC_CTRL_EOFM;
	reg_write(jzfb, LCDC_CTRL, reg);
}

static inline void jzfb_lcdc_disable_frame_end_irq(struct jzfb *jzfb) {
	unsigned int reg = reg_read(jzfb, LCDC_CTRL);
	reg &= ~LCDC_CTRL_EOFM;
	reg_write(jzfb, LCDC_CTRL, reg);
}

static inline int jzfb_lcdc_frame_end_irq_is_enable(struct jzfb *jzfb) {
	return reg_read(jzfb, LCDC_CTRL) & LCDC_CTRL_EOFM;
}

struct intc_regs {
	volatile unsigned int ICSR0;
	volatile unsigned int ICMR0;
	volatile unsigned int ICMSR0;
	volatile unsigned int ICMCR0;
	volatile unsigned int ICPR0;
	volatile unsigned int reserved[3];
	volatile unsigned int ICSR1;
	volatile unsigned int ICMR1;
	volatile unsigned int ICMSR1;
	volatile unsigned int ICMCR1;
	volatile unsigned int ICPR1;
	volatile unsigned int DSR0;
	volatile unsigned int DMR0;
	volatile unsigned int DPR0;
	volatile unsigned int DSR1;
	volatile unsigned int DMR1;
	volatile unsigned int DPR1;
};

#define INTC_IO_BASE 0xB0001000

static inline void jz4775_intc_clear_lcd(void) {
	struct intc_regs *intc = (void *)INTC_IO_BASE;

	intc->ICSR0 &= ~(1 << 31);
	intc->ICPR0 &= ~(1 << 31);

	intc->DSR0 &= ~(1 << 31);
	intc->DPR0 &= ~(1 << 31);
}

static inline void jz4775_intc_irq_unmask(unsigned int mask) {
	struct intc_regs *intc = (void *)INTC_IO_BASE;

	intc->ICMCR0 = mask;
}

static inline void jz4775_intc_irq_dma_unmask(unsigned int mask) {
	struct intc_regs *intc = (void *)INTC_IO_BASE;

	intc->DMR0 &= ~mask;
}

static inline void jz4775_intc_irq_mask(unsigned int mask) {
	struct intc_regs *intc = (void *)INTC_IO_BASE;

	intc->ICMSR0 = mask;
}

static inline void jz4775_intc_irq_dma_mask(unsigned int mask) {
	struct intc_regs *intc = (void *)INTC_IO_BASE;

	intc->DMR0 |= mask;
}

static inline void jz4775_intc_clear(void) {
	struct intc_regs *intc = (void *)INTC_IO_BASE;

	intc->ICSR0 = 0;
	intc->ICPR0 = 0;

	intc->DSR0 = 0;
	intc->DPR0 = 0;
}

extern void do_idle(void);

struct save_intc_mask {
	unsigned int ICMR0;			/* 1 IS CLEAR */
	unsigned int ICMR1;
	unsigned int DMR0;			/* 1 IS CLEAR */
	unsigned int DMR1;
};

static inline void jz4775_intc_mask_all_save(struct save_intc_mask *save) {
	struct intc_regs *intc = (void *)INTC_IO_BASE;

	save->DMR0 = intc->DMR0;
	save->DMR1 = intc->DMR1;

	save->ICMR0 = intc->ICMR0;
	save->ICMR1 = intc->ICMR1;

	intc->DMR0 = 0xffffffff;
	intc->DMR1 = 0xffffffff;
	intc->ICMR0 = 0xffffffff;
	intc->ICMR1 = 0xffffffff;
}

static inline void jz4775_intc_mask_restore(struct save_intc_mask *save) {
	struct intc_regs *intc = (void *)INTC_IO_BASE;

	intc->DMR0  = save->DMR0;
	intc->DMR1  = save->DMR1;

	intc->ICMR0 = save->ICMR0;
	intc->ICMR1 = save->ICMR1;
}

void ddr_start_cycle_count(void) {
	*((volatile unsigned int *)0xb00000d0) = 0x7d;
	*((volatile unsigned int *)0xb34f00d4) = 0;
	*((volatile unsigned int *)0xb34f00d8) = 0;
	*((volatile unsigned int *)0xb34f00dc) = 0;
	*((volatile unsigned int *)0xb34f00e4) = 3;
}

void ddr_end_cycle_count(void) {
	unsigned i,j,k;

	*((volatile unsigned int *)0xb34f00e4) = 2;
	i = *((volatile unsigned int *)0xb34f00d4);
	j = *((volatile unsigned int *)0xb34f00d8);
	k = *((volatile unsigned int *)0xb34f00dc);

	pr_err("total_cycle = %d,valid_cycle = %d\n",i,j);
	pr_err("rate      = %%%d\n",j * 100 / i);
	pr_err("idle_rate = %%%d\n\n",k * 100 / i);
}

static unsigned int cpccr_save = 0;

void cpu_clk_switch_to_extclk(void) {
	unsigned int tmp;

	cpccr_save = tmp = *((volatile unsigned int *)(0xb0000000 + 0x00));
	tmp = (tmp & ~(0x3 << 30)) | (2 << 30);
	*((volatile unsigned int *)(0xb0000000 + 0x00)) = tmp;
	while (*((volatile unsigned int *)(0xb0000000 + 0xD4)) & 0x1);
}

void cpu_clk_switch_back_to_appll(void) {
	*((volatile unsigned int *)(0xb0000000 + 0x00)) = cpccr_save | (1 << 22);
	while (*((volatile unsigned int *)(0xb0000000 + 0xD4)) & 0x1);
}

void jzfb_lcdc_wait_dma_end(struct jzfb *jzfb) {
	unsigned int count = 1000;
	unsigned int count2 = 1000;

#ifdef CONFIG_M200
	struct save_intc_mask save;

	if ((reg_read(jzfb, LCDC_STATE) & LCDC_STATE_EOF))
		goto wait_slcd;

#if 1
	jz4775_intc_clear_lcd();
#else
	jz4775_intc_clear();
#endif
	jz4775_intc_mask_all_save(&save);
	jz4775_intc_irq_unmask(1 << 17);
	jz4775_intc_irq_unmask(1 << 31);
	jz4775_intc_irq_dma_unmask(1 << 31);

//	ddr_start_cycle_count();

	cpu_clk_switch_to_extclk();
	do_idle();
	cpu_clk_switch_back_to_appll();

//	ddr_end_cycle_count();

	jz4775_intc_clear_lcd();
	jz4775_intc_irq_mask(1 << 31);
	jz4775_intc_irq_dma_mask(1 << 31);
	jz4775_intc_mask_restore(&save);
	while (count-- && !(reg_read(jzfb, LCDC_STATE) & LCDC_STATE_EOF)) {
		udelay(10);
	}

wait_slcd:
	while (count2-- && (reg_read(jzfb, SLCDC_STATE) & SLCDC_STATE_BUSY)) {
		udelay(10);
	}
#else
	/* mdelay(1); */

	while (count-- && !(reg_read(jzfb, LCDC_STATE) & LCDC_STATE_EOF)) {
		udelay(10);
	}
	while (count2-- && (reg_read(jzfb, SLCDC_STATE) & SLCDC_STATE_BUSY)) {
		udelay(10);
	}
#endif

	pr_debug("count: %u count2: %u\n", count, count2);
}

void print_lcdc_state(struct jzfb *jzfb) {
	unsigned int busy = reg_read(jzfb, SLCDC_STATE) & SLCDC_STATE_BUSY;
	unsigned int lcdstate = reg_read(jzfb, LCDC_STATE);
	unsigned int osdstate = reg_read(jzfb, LCDC_OSDS);
	pr_info("slcdstate busy: %x\n", busy);
	pr_info("lcdstate: %x\n", lcdstate);
	pr_info("osdstate: %x\n", osdstate);
}

int jzfb_dma_transfer(struct jzfb *jzfb, unsigned int count) {
	unsigned int i;

	jzfb_clk_enable(jzfb);

	for (i = 0; i < count; ++i) {
		jzfb_lcdc_clear_state(jzfb);
		jzfb_lcdc_start_slcd_dma(jzfb);
		jzfb_lcdc_wait_dma_end(jzfb);
	}

	return 0;
}

static inline int jzfb_dma_transfer_one(struct jzfb *jzfb) {

	jzfb_lcdc_clear_state(jzfb);
	jzfb_lcdc_enable_frame_end_irq(jzfb);
	jzfb_lcdc_start_slcd_dma(jzfb);
	/* jzfb_lcdc_wait_dma_end(jzfb); */

	return 0;
}

int jzfb_replace_kernel(struct jzfb *jzfb) {
	volatile struct jzfb_framedesc *fbd;

	jzfb_clk_enable(jzfb);

	fbd = (void *)(0xA0000000 | reg_read(jzfb, LCDC_DA0));
	print_framedesc((struct jzfb_framedesc *)fbd);

	if (fbd->cmd & LCDC_CMD_CMD) {
		pr_info("jzfb: is cmd, check to next\n");
		fbd = (void *)(0xA0000000 | fbd->next);
		print_framedesc((struct jzfb_framedesc *)fbd);
	}

	jzfb->save.base = (unsigned long)fbd->databuf;
	fbd->databuf = (u32)jzfb->fbs.base_phys;

	print_framedesc((struct jzfb_framedesc *)fbd);

	return 0;
}

int jzfb_restore_kernel(struct jzfb* jzfb) {
	volatile struct jzfb_framedesc *fbd;

	jzfb_clk_enable(jzfb);

	fbd = (void *)(0xA0000000 | reg_read(jzfb, LCDC_DA0));
	print_framedesc((struct jzfb_framedesc *)fbd);

	if (fbd->cmd & LCDC_CMD_CMD) {
		pr_info("jzfb: is cmd, check to next\n");
		fbd = (void *)(0xA0000000 | fbd->next);
		print_framedesc((struct jzfb_framedesc *)fbd);
	}

	fbd->databuf = jzfb->save.base;

	print_framedesc((struct jzfb_framedesc *)fbd);

	return 0;
}

void jzfb_set_transfer_mode(struct jzfb *jzfb, unsigned int mode) {
	jzfb->s.is_gpio_transfer = mode;

	jzfb_lcdc_init_gpio(jzfb);
}

static inline int jzfb_pan_dispaly_internal(struct jzfb *jzfb, unsigned int offset) {
	struct fb_struct *fbs = &jzfb->fbs;
	unsigned int n = offset / fbs->yres;
	int ret = 0;

	if (n >= fbs->nums) {
		pr_err("jzfb: error: offset:%u, max:%u\n", offset, fbs->nums * fbs->yres);
		return -EINVAL;
	}

	if (jzfb->buffer_index != n) {
		jzfb_set_base(jzfb, fbs->base_phys + n * fbs->size);
		jzfb->buffer_index = n;
	}

	if (jzfb->pdata->lcd_type == LCD_TYPE_LCM) {
		if (jzfb->s.is_gpio_transfer) {
			ret = jzfb_gpio_flush_lcd(jzfb, fbs->base + n * fbs->size);
		} else {
#ifdef CONFIG_M200
#ifdef CONFIG_FB_DOUBLE_BUFFERING
			protected_blast_dcache_range((unsigned long)fbs->base + n * fbs->size, (unsigned long)fbs->base + (n ) * fbs->size + fbs->size / 20);
			protected_blast_scache_range((unsigned long)fbs->base + n * fbs->size, (unsigned long)fbs->base + (n ) * fbs->size + fbs->size / 20);
#else
			protected_blast_dcache_range((unsigned long)fbs->base + n * fbs->size, (unsigned long)fbs->base + (n + 1) * fbs->size);
			protected_blast_scache_range((unsigned long)fbs->base + n * fbs->size, (unsigned long)fbs->base + (n + 1) * fbs->size);
#endif
			asm volatile ("sync");
#else
			flush_cache((unsigned long)fbs->base + n * fbs->size, (n + 1) * fbs->size);
#endif
			jzfb_dma_transfer_one(jzfb);
		}
	}

	return ret;
}

int jzfb_pan_dispaly(struct jzfb *jzfb, unsigned int offset) {
	int ret;

	if (!jzfb->s.is_enabled) {
		ret = jzfb_enable(jzfb);
		if (ret) {
			pr_err("jzfb: error: jzfb init failed\n");
			return ret;
		}
	}

	return jzfb_pan_dispaly_internal(jzfb, offset);
}

int jzfb_flush_fb(struct jzfb *jzfb, unsigned int offset) {
	struct fb_struct *fbs = &jzfb->fbs;
	unsigned int n = offset / fbs->yres;

	if (n >= fbs->nums) {
		pr_err("jzfb: flush fb out of buffer index!\n");
		return -EINVAL;
	}

#ifdef CONFIG_M200
	protected_blast_dcache_range((unsigned long)fbs->base + n * fbs->size, (unsigned long)fbs->base + n * fbs->size + fbs->size);
	protected_blast_scache_range((unsigned long)fbs->base + n * fbs->size, (unsigned long)fbs->base + n * fbs->size + fbs->size);
#else
	flush_cache((unsigned long)fbs->base + n * fbs->size, fbs->size);
#endif

	return 0;
}

int jzfb_flush_cache(struct jzfb *jzfb) {
	struct fb_struct *fbs = &jzfb->fbs;

#ifdef CONFIG_M200
	blast_dcache_range((unsigned long)fbs->base, fbs->nums * fbs->size);
	blast_scache_range((unsigned long)fbs->base, fbs->nums * fbs->size);
#else
	flush_cache((unsigned long)fbs->base, fbs->nums * fbs->size);
#endif

	return 0;
}

int jzfb_init_data(struct jzfb *jzfb, struct jzfb_platform_data *pdata) {
	struct fb_videomode *mode;
	int ret;
	int pixel_align = -1;

	if (!jzfb || !pdata) {
		pr_err("jzfb: error: pdata and jzfb struct can not be none\n");
		return -EINVAL;
	}

	pixel_align = slpt_get_kernel_pixel_align();
	if(pixel_align > 0) {
		jzfb_pixel_align = pixel_align;
		slpt_kernel_printf("get kernel pixel align is ok\n");
	}

	jzfb->s.is_inited = 0;
	jzfb->s.is_enabled = 0;
	jzfb->s.is_panel_setted = 0;
	jzfb->s.is_gpio_transfer = CONFIG_JZFB_GPIO_FLUSH_LCD;
	jzfb->s.need_to_init_gpio = 1;
	jzfb->s.need_to_save_base = 1;
	jzfb->s.is_clk_enabled = 0;
	jzfb->s.need_to_enable_fb = 1;
	jzfb->s.need_to_enable_clk = 1;

	jzfb->pdata = pdata;
	jzfb->slcd_mode = pdata->slcd_mode;
	jzfb->id = 0;
	jzfb->base = LCDC0_IOBASE;

	if (!pdata->num_modes || !pdata->modes) {
		pr_err("jzfb: error: videomode can not be none\n");
		return -EINVAL;
	}
	mode = pdata->modes; /* using the first videmode */

	if (!mode->xres || !mode->yres || !mode->pixclock || !pdata->bpp) {
		pr_err("jzfb: error: invalid videmode: xres:%u yres:%u pixclock:%u bpp:%u\n",
			   mode->xres, mode->yres, mode->pixclock, pdata->bpp);
		return -EINVAL;
	}

	if (!jzfb_supported_lcd_type(pdata->lcd_type)) {
		pr_err("jzfb: error: currently not support this lcd type:%u\n", pdata->lcd_type);
		return -ENODEV;
	}

	jzfb->dsi.address = DSI_BASE;
	jzfb->dsi.state = pdata->mipi_dsi ? INITIALIZED : NOT_INITIALIZED;

	jzfb->mode = mode;
	ret = jzfb_alloc_mem(jzfb);
	if (ret) {
		pr_err("jzfb: error: allocate fb memory failed\n");
		return ret;
	}

	FBS_TO_REGION(jzfb->fbs, jzfb->region);
	set_current_fb_region(&jzfb->region);

	jzfb->s.is_inited = 1;

	return 0;
}

static unsigned int ddr_auto_self_refresh_count_save = 0;

int jzfb_update_status(struct jzfb *jzfb) {
	unsigned int enable = jzfb->s.is_clk_enabled;

	ddr_auto_self_refresh_count_save = *((volatile unsigned int *)0xB34f0308);
	*((volatile unsigned int *)0xB34f0308) =  32;

	pr_debug("ddr_auto_self_refresh_count_save = %u\n", ddr_auto_self_refresh_count_save);

	jzfb_clk_enable(jzfb);

	jzfb_save_base(jzfb);

	jzfb->s.need_to_enable_irq = jzfb_lcdc_frame_end_irq_is_enable(jzfb);

	jzfb_lcdc_enable_frame_end_irq(jzfb);

	if (!enable) {
		jzfb_clk_disable(jzfb);
	}

	jzfb->buffer_index = -1;

	return 0;
}

void jzfb_copy_buf_to_kernel(struct jzfb *jzfb) {
	void *fbmem = (void *)slpt_kernel_get_jzfb_param(jzfb->id, "fb-mem");
	unsigned int fb_num = (unsigned int) slpt_kernel_get_jzfb_param(jzfb->id, "fb-num");
	unsigned int frmsize = (unsigned int) slpt_kernel_get_jzfb_param(jzfb->id, "frmsize");
	int i;

	pr_info("jzfb: %s info: (%p %d %d) (%d)\n", __FUNCTION__, fbmem, fb_num, frmsize, jzfb->fbs.size);

	if (fbmem && fb_num && (frmsize == jzfb->fbs.size)) {
		for (i = 0; i < fb_num; ++i) {
			memcpy(fbmem + frmsize * i, jzfb->fbs.base, frmsize);
		}
	}
}

int jzfb_restore_status(struct jzfb *jzfb) {

	*((volatile unsigned int *)0xB34f0308) = ddr_auto_self_refresh_count_save;

	jzfb_enable(jzfb);

	jzfb_restore_base(jzfb);

	jzfb_copy_buf_to_kernel(jzfb);

	/* jzfb_lcdc_start_slcd_dma(jzfb); */

	if (jzfb->s.need_to_enable_irq)
		jzfb_lcdc_enable_frame_end_irq(jzfb);
	else
		jzfb_lcdc_disable_frame_end_irq(jzfb);

	if (!jzfb->s.need_to_enable_fb) {
		jzfb_disable(jzfb);
	}

	if (jzfb->s.need_to_enable_fb) {
		jzfb_clk_enable(jzfb);
	}

	return 0;
}
