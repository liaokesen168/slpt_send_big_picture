/*
 * Jz4780 UART support
 * 	base on Jz4740 uart driver
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co., Ltd.
 * Authors: Kage Shen <kkshen@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 3 of the License, or (at your option) any later version.
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <serial.h>
#include <asm/arch/uart.h>
#include <linux/compiler.h>

#include <asm/arch/board_special.h>

#if defined(CONFIG_JZ4780)
#include <asm/arch/jz4780_gpio.h>
#elif defined(CONFIG_JZ4775)
#include <asm/arch/jz4775_gpio.h>
#elif defined(CONFIG_M200)
#include <asm/arch/m200_gpio.h>
#endif

/*
 * serial_init - initialize a channel
 *
 * This routine initializes the number of data bits, parity
 * and set the selected baud rate. Interrupts are disabled.
 * Set the modem control signals if the option is selected.
 *
 * RETURNS: N/A
 */

static struct jz4780_uart_regs *uart_regs =
		(struct jz4780_uart_regs *) UART3_BASE;

int uart_init(void)
{
	/* FIXME: add get uart parameter func */

	switch(misc_param.uart_num) {
	case 3:
		uart_regs = (struct jz4780_uart_regs *) UART3_BASE;
		break;
	case 2:
		uart_regs = (struct jz4780_uart_regs *) UART2_BASE;
		break;
	case 1:
		uart_regs = (struct jz4780_uart_regs *) UART1_BASE;
		break;
	case 0:
		uart_regs = (struct jz4780_uart_regs *) UART0_BASE;
		break;
	}

#ifndef CONFIG_SLPT
	gpio_as_uart3_jtag();
#endif

	return 0;
}


static void uart_setbrg(void)
{

#if 0
#define CFG_EXTAL (24 * 1000000)
	volatile u8 *uart_lcr = (volatile u8 *) (0xB0033000 + OFF_LCR);
	volatile u8 *uart_dlhr = (volatile u8 *) (0xB0033000 + OFF_DLHR);
	volatile u8 *uart_dllr = (volatile u8 *) (0xB0033000 + OFF_DLLR);
	volatile u8 *uart_umr = (volatile u8 *) (0xB0033000 + OFF_UMR);
	volatile u8 *uart_uacr = (volatile u8 *) (0xB0033000 + OFF_UACR);
	volatile u16 baud_div, tmp;

//	uart_regs->lcr
	*uart_umr = 16;
	*uart_uacr = 0;

	/* EXTAL freq: 12MHz, 24MHz, 48MHz */
#if CFG_EXTAL ==(12 * 1000000) || CFG_EXTAL ==(24 * 1000000) || CFG_EXTAL ==(48 * 1000000)
	baud_div = 13 * (CFG_EXTAL / (12 * 1000000)); /* 12MHz Crystall, 57600, baud_div = 13; */
#else
#error "check baud_div"
#endif

	tmp = *uart_lcr;
	tmp |= UART_LCR_DLAB;
	*uart_lcr = tmp;

	*uart_dlhr = (baud_div >> 8) & 0xff;
	*uart_dllr = baud_div & 0xff;

	tmp &= ~UART_LCR_DLAB;
	*uart_lcr = tmp;

//	*uart_dlhr |= 1;			// enable receive interrupt
#endif

	return;
}

void jz47xx_set_uart(void)
{
	uart_setbrg();
}


static int uart_tstc(void)
{
	if(readb(&uart_regs->urcr) != 0)
		return 1;
	return 0;
}

static void uart_putc(const char c)
{
	if (c == '\n')
		uart_putc('\r');
	/* Wait for fifo to shift out some bytes */
	while (!((readb(&uart_regs->lsr) &
			(UART_LSR_TDRQ | UART_LSR_TEMT)) == 0x60));
	writeb(c, &uart_regs->regs0.tdr);
}

static int uart_getc(void)
{
	while (!uart_tstc());
	return readb(&uart_regs->regs0.rdr);
}

void uart_puts(const char *s)
{
	while (*s) {
		uart_putc(*s++);
	}
}

static struct serial_device uart_drv = {
		.name = "jz4780_uart",
		.start = uart_init,
		.stop = NULL,
		.setbrg = uart_setbrg,
		.putc = uart_putc,
		.puts = default_serial_puts,
		.getc = uart_getc,
		.tstc = uart_tstc,
};

void jz4780_serial_initialize(void)
{
	uart_init();
	serial_register(&uart_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &uart_drv;
}

void tcsm_put_hex(unsigned int d)
{
	register unsigned int i;
	register unsigned int c;

	TCSM_PCHAR('*');
	for (i = 0; i < 8; i++) {
		c = (d >> ((7 - i) * 4)) & 0xf;
		if (c < 10)
			c += 0x30;
		else
			c += (0x41 - 10);

		TCSM_PCHAR(c);
	}
	TCSM_PCHAR('\n');
	TCSM_PCHAR('\r');

	i = 1;
	while (i--)
		;
}
