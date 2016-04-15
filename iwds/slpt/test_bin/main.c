#include "types.h"
#include "slpt.h"

#define __raw_readb(addr) (*(volatile unsigned char *)(addr))
#define __raw_readw(addr) (*(volatile unsigned short *)(addr))
#define __raw_readl(addr) (*(volatile unsigned int *)(addr))
#define readb(addr) __raw_readb((addr))
#define readw(addr) __ioswab16(__raw_readw((addr)))
#define readl(addr) __ioswab32(__raw_readl((addr)))

#define __raw_writeb(b, addr) (*(volatile unsigned char *)(addr)) = (b)
#define __raw_writew(b, addr) (*(volatile unsigned short *)(addr)) = (b)
#define __raw_writel(b, addr) (*(volatile unsigned int *)(addr)) = (b)
#define writeb(b, addr) __raw_writeb((b), (addr))
#define writew(b, addr) __raw_writew(__ioswab16(b), (addr))
#define writel(b, addr) __raw_writel(__ioswab32(b), (addr))

#define memset_io(a,b,c)	memset((void *)(a),(b),(c))
#define memcpy_fromio(a,b,c)	memcpy((a),(void *)(b),(c))
#define memcpy_toio(a,b,c)	memcpy((void *)(a),(b),(c))

#define	UART0_BASE	0xB0030000
#define	UART1_BASE	0xB0031000
#define	UART2_BASE	0xB0032000
#define	UART3_BASE	0xB0033000
#define	UART4_BASE	0xB0034000

/*
 * Define macros for UART_LSR
 * UART Line Status Register
 */
#define UART_LSR_DR	(1 << 0)		/* 0: receive FIFO is empty  1: receive data is ready */
#define UART_LSR_ORER	(1 << 1)	/* 0: no overrun error */
#define UART_LSR_PER	(1 << 2)	/* 0: no parity error */
#define UART_LSR_FER	(1 << 3)	/* 0; no framing error */
#define UART_LSR_BRK	(1 << 4)	/* 0: no break detected  1: receive a break signal */
#define UART_LSR_TDRQ	(1 << 5)	/* 1: transmit FIFO half "empty" */
#define UART_LSR_TEMT	(1 << 6)	/* 1: transmit FIFO and shift registers empty */
#define UART_LSR_RFER	(1 << 7)	/* 0: no receive error  1: receive error in FIFO mode */

struct jz4780_uart_regs {
	union {
		volatile u32 rdr;
		volatile u32 tdr;
		volatile u32 dllr;
	} regs0; /* 00 */
	union {
		volatile u32 ier;
		volatile u32 dlhr;
	} regs4; /* 04 */
	union {
		volatile u32 isr;
		volatile u32 fcr;
	} regs8; /* 08 */

	volatile u32 lcr; 	/* 0c */
	volatile u32 mcr; 	/* 10 */
	volatile u32 lsr; 	/* 14 */
	volatile u32 msr; 	/* 18 */
	volatile u32 spr; 	/* 1c */
	volatile u32 sircr; /* 20 */
	volatile u32 umr; 	/* 24 */
	volatile u32 uacr; 	/* 28 - 16b */
	volatile u32 padding0[(0x40 - 0x2c) / sizeof(u32)];
	volatile u32 urcr; 	/* 40 */
	volatile u32 utcr; 	/* 44 */
};

static struct jz4780_uart_regs *uart_regs;

static void uart_putc(const char c)
{
	if (c == '\n')
		uart_putc('\r');
	/* Wait for fifo to shift out some bytes */
	while (!((readb(&uart_regs->lsr) &
			(UART_LSR_TDRQ | UART_LSR_TEMT))== 0x60));

	writeb(c, &uart_regs->regs0.tdr);
}

static void uart_puts(const char *s)
{
	while (*s) {
		uart_putc(*s++);
	}
}

static void uart_init(void)
{
	uart_regs = (struct jz4780_uart_regs *) UART3_BASE;
}

void slpt_main(unsigned long api_addr, struct slpt_task *task)
{
	uart_init();
	uart_puts("\nMcu: System init OK!");
	slpt_pm_enter(0);
}


char slpt_test_mem[] = "this just a test str from task\n";

int slpt_init_f(unsigned long api_addr, struct slpt_task *task) {
	struct slpt_app_res res_t = {
		.name = "test-mem",
		.type = SLPT_RES_MEM,
		.addr = slpt_test_mem,
		.length = sizeof(slpt_test_mem),
	};
	struct slpt_app_res *res;

	slpt_app_get_api_val = (void *)api_addr;

	slpt_printf("SLPT: info: %s is called\n", __FUNCTION__);

	res = slpt_app_register_res(&res_t, task);
	if (!res) {
		slpt_printf("SLPT: error: failed to register res mem-test\n");
		return -1;
	}

	return 0;
}

int slpt_exit(unsigned long api_addr, struct slpt_task *task) {
	struct slpt_app_res *res;

	res = name_to_slpt_app_res("test-mem", task);
	if (!res) {
		slpt_printf("SLPT: error: failed to get test-mme res\n");
	} else {
		slpt_app_unregister_res(res, task);
	}
	slpt_printf("SLPT: info: %s is called\n", __FUNCTION__);

	return 0;
}
