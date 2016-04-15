/*
 *  Copyright (C) 2013 Fighter Sun <wanmyqawdr@126.com>
 *  HW ECC-BCH support functions
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

#include <config.h>
#include <common.h>
#include <linux/err.h>

#include <asm/addrspace.h>
#include <asm/arch/cpm.h>
#include <asm/arch/bch.h>
#include <malloc.h>
#define CONFIG_JZ4780_BCH_USE_PIO

#define DRVNAME "jz4780-bch"

#define BCH_INT_DECODE_FINISH	(1 << 3)
#define BCH_INT_ENCODE_FINISH	(1 << 2)
#define BCH_INT_UNCORRECT	(1 << 1)

#define BCH_ENABLED_INT	\
	(	\
	BCH_INT_DECODE_FINISH |	\
	BCH_INT_ENCODE_FINISH |	\
	BCH_INT_UNCORRECT	\
	)

#define BCH_CLK_RATE (200 * 1000 * 1000)

#define BCH_REGS_FILE_BASE	0x134d0000

#define CNT_PARITY  28
#define CNT_ERRROPT 64

typedef struct {
	volatile u32 bhcr;
	volatile u32 bhcsr;
	volatile u32 bhccr;
	volatile u32 bhcnt;
	volatile u32 bhdr;
	volatile u32 bhpar[CNT_PARITY];
	volatile u32 bherr[CNT_ERRROPT];
	volatile u32 bhint;
	volatile u32 bhintes;
	volatile u32 bhintec;
	volatile u32 bhinte;
	volatile u32 bhto;
} regs_file_t;

/* instance a singleton bchc */
struct {
	struct clk *clk_bch;

	regs_file_t * const regs_file;

	u32 saved_reg_bhint;
} instance = {
	.regs_file = (regs_file_t *)
					CKSEG1ADDR(BCH_REGS_FILE_BASE),
}, *bchc = &instance;

inline static void bch_select_encode(bch_request_t *req)
{
	bchc->regs_file->bhcsr = 1 << 2;
}

inline static void bch_select_ecc_level(bch_request_t *req)
{
	bchc->regs_file->bhccr = 0x7f << 4;
	bchc->regs_file->bhcsr = req->ecc_level << 4;
}

inline static void bch_select_calc_size(bch_request_t *req)
{
	bchc->regs_file->bhcnt = 0;
	bchc->regs_file->bhcnt = (req->parity_size << 16) | req->blksz;
}

inline static void bch_select_decode(bch_request_t *req)
{
	bchc->regs_file->bhccr = 1 << 2;
}

inline static void bch_clear_pending_interrupts(void)
{
	/* clear enabled interrupts */
	bchc->regs_file->bhint = BCH_ENABLED_INT;
}

inline static void bch_wait_for_encode_done(bch_request_t *req)
{
	do {
		if (bchc->regs_file->bhint &
				BCH_INT_ENCODE_FINISH)
			goto done;
	} while (1);

done:
	bchc->saved_reg_bhint = bchc->regs_file->bhint;
	bch_clear_pending_interrupts();
}

inline static void bch_wait_for_decode_done(bch_request_t *req)
{
	do {
		if (bchc->regs_file->bhint &
				(BCH_INT_DECODE_FINISH |
						BCH_INT_UNCORRECT))
			goto done;
	} while (1);

done:
	bchc->saved_reg_bhint = bchc->regs_file->bhint;
	bch_clear_pending_interrupts();
}

inline static void bch_start_new_operation(void)
{
	/* start operation */
	bchc->regs_file->bhcsr = 1 << 1;
}

#ifdef CONFIG_JZ4780_BCH_USE_PIO

inline static void write_data_by_cpu(const void *data, u32 size)
{
	int i = size / sizeof(u32);
	int j = size & 0x3;

	volatile void *dst = &bchc->regs_file->bhdr;
	const u32 *src32;
	const u8 *src8;

	if (((u32)data & 0x3) == 0) {
		src32 = (u32 *)data;
		while (i--)
			*(u32 *)dst = *src32++;

		src8 = (u8 *)src32;
		while (j--)
			*(u8 *)dst = *src8++;
	} else {
		src8 = (u8 *)data;
		while (size--)
			*(u8 *)dst = *src8++;
	}
}

inline static void read_err_report_by_cpu(bch_request_t *req)
{
	if (unlikely(bchc->saved_reg_bhint & BCH_INT_UNCORRECT)) {
		req->errrept_word_cnt = 0;
		req->cnt_ecc_errors = 0;
		req->ret_val = BCH_RET_UNCORRECTABLE;
		/* printk(DRVNAME" BCH uncorrectable!\n"); */
	} else if (bchc->saved_reg_bhint & BCH_INT_DECODE_FINISH) {
		int i;

		req->errrept_word_cnt = (bchc->saved_reg_bhint
				& (0x7f << 24)) >> 24;
		req->cnt_ecc_errors = (bchc->saved_reg_bhint
				& (0x7f << 16)) >> 16;

		for (i = 0; i < req->errrept_word_cnt; i++)
			req->errrept_data[i] = bchc->regs_file->bherr[i];

		req->ret_val = BCH_RET_OK;
#if 0
		if (req->cnt_ecc_errors)
			printk(DRVNAME" BCH corrected, ecc error bits count=%d\n",
					req->cnt_ecc_errors);
#endif
	} else {
		req->errrept_word_cnt = 0;
		req->cnt_ecc_errors = 0;
		req->ret_val = BCH_RET_UNEXPECTED;
		printk(DRVNAME" BCH unexpected!\n");
	}
}

inline static void read_parity_by_cpu(bch_request_t *req)
{
	if (likely(bchc->saved_reg_bhint & BCH_INT_ENCODE_FINISH)) {
		int i = req->parity_size / sizeof(u32);
		int j = req->parity_size & 0x3;
		u32 *ecc_data32;
		u8 *ecc_data8, *ptr;
		volatile u32 *parity32;

		u32 *buffer = malloc(req->parity_size);

		if (!buffer) {
			printk(DRVNAME" No memory for bch!!\n");
			BUG();
		}

		ecc_data32 = buffer;
		ecc_data8 = (u8 *)buffer;
		ptr = (u8 *)req->ecc_data;
		parity32 = bchc->regs_file->bhpar;

		while (i--)
			*ecc_data32++ = *parity32++;

		if (j)
			*ecc_data32++ = *parity32++;

		for(i = 0; i < req->parity_size; i++)
			*ptr++ = *ecc_data8++; 

		free(buffer);
		req->ret_val = BCH_RET_OK;
	} else {
		req->ret_val = BCH_RET_UNEXPECTED;
	}
}

#else

/* TODO: fill them */

inline static void bch_dma_config(void)
{

}

inline static void write_data_by_dma(const void *data, u32 size)
{

}

inline static void read_err_report_by_dma(bch_request_t *req)
{

}

inline static void read_parity_by_dma(bch_request_t *req)
{

}

#endif

void bch_encode(bch_request_t *req)
{
	/*
	 * step1. basic config
	 */
	bch_select_encode(req);
	bch_select_ecc_level(req);
	bch_select_calc_size(req);

	/*
	 * step2.
	 */
	bch_start_new_operation();


#ifdef CONFIG_JZ4780_BCH_USE_PIO
	/*
	 * step3. transfer raw data which to be encoded
	 */
	write_data_by_cpu(req->raw_data, req->blksz);

	/*
	 * step4.
	 */
	bch_wait_for_encode_done(req);

	/*
	 * step5. read out parity data
	 */
	read_parity_by_cpu(req);

#else

	/*
	 * TODO
	 */
	#error "TODO: implement DMA transfer"


#endif
}

void bch_decode(bch_request_t *req)
{
	/*
	 * step1. basic config
	 */
	bch_select_ecc_level(req);
	bch_select_decode(req);
	bch_select_calc_size(req);

	/*
	 * step2.
	 */
	bch_start_new_operation();

#ifdef CONFIG_JZ4780_BCH_USE_PIO
	/*
	 * step3. transfer raw data which to be decoded
	 */
	write_data_by_cpu(req->raw_data, req->blksz);

	/*
	 * step4. following transfer ECC code
	 */
	write_data_by_cpu(req->ecc_data, req->parity_size);

	/*
	 * step5.
	 */
	bch_wait_for_decode_done(req);

	/*
	 * step6. read out error report data
	 */
	read_err_report_by_cpu(req);

#else

	/*
	 * TODO
	 */
	#error "TODO: implement DMA transfer"

#endif
}

void bch_correct(bch_request_t *req)
{
	int i;
	int mask;
	int index;
	u16 *raw_data;

	raw_data = (u16 *)req->raw_data;
	for (i = 0; i < req->errrept_word_cnt; i++) {
		index = req->errrept_data[i] & 0xffff;
		mask = (req->errrept_data[i] & (0xffff << 16)) >> 16;
		raw_data[index] ^= mask;
	}

	req->ret_val = BCH_RET_OK;
}

void bch_decode_correct(bch_request_t *req)
{
	/* start decode process */
	bch_decode(req);

	/* return if req is not correctable */
	if (req->ret_val)
		return;

	/* start correct process */
	bch_correct(req);
}

inline static void bch_pio_config(void)
{
	bchc->regs_file->bhccr = 1 << 11;
}

inline static void bch_enable(void)
{
	/* enable bchc */
	bchc->regs_file->bhcsr = 1;

	/* do not bypass decoder */
	bchc->regs_file->bhccr = 1 << 12;
}

int bch_request_submit(bch_request_t *req)
{
	if (req->blksz & 0x3 || req->blksz > 1900)
		return -EINVAL;
	else if (req->ecc_level & 0x3 || req->ecc_level > 64)
		return -EINVAL;

	req->parity_size = req->ecc_level * 14 >> 3;

	switch (req->type) {
	case BCH_REQ_ENCODE:
		bch_encode(req);
		break;

	case BCH_REQ_DECODE:
		bch_decode(req);
		break;

	case BCH_REQ_DECODE_CORRECT:
		bch_decode_correct(req);
		break;

	case BCH_REQ_CORRECT:
		bch_correct(req);
		break;

	default:
		printk(DRVNAME" BCH unknown request type: %d \n", req->type);
		req->ret_val = BCH_RET_UNSUPPORTED;
		break;
	}
	return 0;
}

static int bch_clk_config(void)
{
	bchc->clk_bch = clk_get("bch");
	if (IS_ERR(bchc->clk_bch)) {
		printk(DRVNAME" Failed to request clk cgu_bch\n");

		return PTR_ERR(bchc->clk_bch);
	}

	/* TODO: consider variable clk rate */
	clk_disable(bchc->clk_bch);
	clk_set_rate(bchc->clk_bch, BCH_CLK_RATE);
	clk_enable(bchc->clk_bch);

	return 0;
}

static void bch_irq_config(void)
{
	bchc->regs_file->bhintec = 0;
	bchc->regs_file->bhint = ~(u32)0;
}

int bch_init(void)
{
	int ret;

	ret = bch_clk_config();
	if (ret)
		return ret;

	bch_enable();

#ifdef CONFIG_JZ4780_BCH_USE_PIO
	bch_pio_config();
#else
	bch_dma_config();
#endif

	bch_irq_config();

	printk(DRVNAME" SoC-jz4780 HW ECC-BCH support "
			"functions initialized.\n");

	return ret;
}

/*
 * MODULE_AUTHOR("Fighter Sun <wanmyqawdr@126.com>");
 * MODULE_DESCRIPTION("SoC-jz4780 HW ECC-BCH support functions");
 * MODULE_LICENSE("GPL");
 * MODULE_ALIAS("platform:"DRVNAME);
 */
