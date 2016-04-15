#include <config.h>
#include <asm/errno.h>
#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <slpt.h>
#include <jzfb.h>
#include <malloc.h>
#include <linux/pr_info.h>
#include <linux/lcd_mcu.h>
#include <fb_struct.h>
#include <linux/err.h>
#include <linux/lcd_mcu.h>

#include "regs.h"
#include "cpm.h"

#define CPM_IOBASE 0xb0000000

#undef outl
#undef inl
#define inl(addr) (*(volatile unsigned int *)(addr))
#define outl(value, addr) (*(volatile unsigned int *)(addr) = (value))

#define GATE_LCD 24
#define GATE_DSI 26

static inline void gate_clk_enable(int clkn) {
	unsigned int off = clkn >= 32 ? CPM_CLKGR1 : CPM_CLKGR;
	unsigned int nb = clkn >= 32 ? clkn % 32 : clkn;

	cpm_clear_bit(nb, off);
}

static inline void gate_clk_disable(int clkn) {
	unsigned int off = clkn >= 32 ? CPM_CLKGR1 : CPM_CLKGR;
	unsigned int nb = clkn >= 32 ? clkn % 32 : clkn;

	cpm_set_bit(nb, off);
}

static inline int gate_clk_enabled(int clkn) {
	unsigned int off = clkn >= 32 ? CPM_CLKGR1 : CPM_CLKGR;
	unsigned int nb = clkn >= 32 ? clkn % 32 : clkn;

	return !cpm_test_bit(nb, off);	
}

#define PWC_CTRL_LCD 5
#define PWC_CTRL_DSI 0

#define PWC_WAIT_LCD 21
#define PWC_WAIT_DSI 16

static inline void pwc_enable(int ctrl_bit, int wait_bit, unsigned int off) {
	unsigned int t;

	t = cpm_test_bit(ctrl_bit, off); //t == 0 is power on
	if(t) {
		cpm_clear_bit(ctrl_bit, off);
		while(cpm_test_bit(wait_bit, off));
	}
}

static inline void pwc_disable(int ctrl_bit, int wait_bit, unsigned int off) {
	unsigned int t;

	t = cpm_test_bit(ctrl_bit, off); //t == 0 is power on
	if(!t) {
		cpm_set_bit(ctrl_bit, off);
		while(!cpm_test_bit(wait_bit, off));
	}
}

static inline int pwc_enabled(int ctrl_bit, int wait_bit, unsigned int off) {
	return !cpm_test_bit(ctrl_bit, off); //t == 0 is power on
}

#define CGU_LCD_STOP 26 

static inline void cgu_clk_enable(int stop_bit, unsigned int off) {
	int busy_bit = stop_bit + 1;
	int ce_bit = stop_bit + 2;

	while(cpm_test_bit(busy_bit, off));

	cpm_set_bit(ce_bit, off);
	cpm_clear_bit(stop_bit, off);
	cpm_clear_bit(ce_bit, off);
}

static inline void cgu_clk_disable(int stop_bit, unsigned int off) {
	int busy_bit = stop_bit + 1;
	int ce_bit = stop_bit + 2;

	while(cpm_test_bit(busy_bit, off));

	cpm_set_bit(ce_bit, off);
	cpm_set_bit(stop_bit, off);
	cpm_clear_bit(ce_bit, off);
}

static inline int cgu_clk_enabled(int stop_bit, unsigned int off) {
	return !cpm_test_bit(stop_bit, off);
}

static inline void print_lcd_clk_state(void) {
	pr_info("DSI: %d, %d\n", gate_clk_enabled(GATE_DSI), pwc_enabled(PWC_CTRL_DSI, PWC_WAIT_DSI, CPM_PGR));
	pr_info("LCD: %d, %d\n", gate_clk_enabled(GATE_LCD), pwc_enabled(PWC_CTRL_LCD, PWC_WAIT_LCD, CPM_PGR));
	pr_info("LPCLK: %d\n", cgu_clk_enabled(CGU_LCD_STOP, CPM_LPCDR));
}

void enable_lcd_clk(void) {

	cgu_clk_enable(CGU_LCD_STOP, CPM_LPCDR);

	gate_clk_enable(GATE_LCD);

	pwc_enable(PWC_CTRL_LCD, PWC_WAIT_LCD, CPM_PGR);

	gate_clk_enable(GATE_DSI);

	pwc_enable(PWC_CTRL_DSI, PWC_WAIT_DSI, CPM_PGR);

}

void disable_lcd_clk(void) {

	/* pwc_disable(PWC_CTRL_DSI, PWC_WAIT_DSI, CPM_PGR); */
	gate_clk_disable(GATE_DSI);

	/* pwc_disable(PWC_CTRL_LCD, PWC_WAIT_LCD, CPM_PGR); */
	gate_clk_disable(GATE_LCD);

	cgu_clk_disable(CGU_LCD_STOP, CPM_LPCDR);
}

static int cmd_lcd_dump_clk(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[]) {
	unsigned int clkgr0 = cpm_inl(CPM_CLKGR);
	unsigned int clkgr1 = cpm_inl(CPM_CLKGR1);
	unsigned int spcr0 = cpm_inl(CPM_SPCR0);
	unsigned int lcr = cpm_inl(CPM_LCR);
	unsigned int pgr = cpm_inl(CPM_PGR);
	unsigned int srbc = cpm_inl(CPM_SRBC);
	unsigned int lpcdr = cpm_inl(CPM_LPCDR);


	print_lcd_clk_state();

	pr_info("CLKGR0: \t%08x\n", clkgr0);
	pr_info("CLKGR1: \t%08x\n", clkgr1);
	pr_info("SPCR0: \t%08x\n", spcr0);
	pr_info("LCR: \t%08x\n", lcr);
	pr_info("PGR: \t%08x\n", pgr);
	pr_info("SRBC: \t%08x\n", srbc);
	pr_info("LPCDR: \t%08x\n", lpcdr);

	return 0;
}

U_BOOT_CMD(
	lcd_dump_clk, 10, 1, cmd_lcd_dump_clk,
	"dump lcd clk registers",
	"dump lcd clk registers"
);
