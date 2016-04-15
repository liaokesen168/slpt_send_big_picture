#include <config.h>
#include <asm/errno.h>

#include <common.h>
#include <asm-generic/gpio.h>
#include <linux/lcd_mcu.h>
#include <linux/pr_info.h>
#include <fb_struct.h>

#include <jzfb.h>

#define inl(addr) (*(volatile unsigned int *)(addr))
#define outl(value, addr) (*(volatile unsigned int *)(addr) = (value))

#ifndef if_gpio_free
/* Sorry for GPIO_PA0, for default it is not define the gpio if the gpio num is 0 */
#define gpio_valid(gpio) ((gpio) > 0)

#define if_gpio_direction_output(gpio, value)		\
	do {											\
		if (gpio_valid(gpio))						\
			gpio_direction_output(gpio, value);		\
		pr_debug("gpio: %d to %d\n", gpio, value);	\
	} while (0)

#define if_gpio_request(gpio, label) (gpio_valid(gpio) ? gpio_request(gpio, label) : 0)

#define if_gpio_free(gpio)						\
	do {                                        \
		if (gpio_valid(gpio))					\
			gpio_free(gpio);                    \
	} while (0)

#endif

#define PCPIN     (0xB0010200)
#define PCINT    (0xB0010210)
#define PCINTS    (0xB0010214)
#define PCINTC    (0xB0010218)

#define PCMASK	  (0xB0010220)
#define PCMASKS	  (0xB0010224)
#define PCMASKC	  (0xB0010228)

#define PCPAT1   (0xB0010230)
#define PCPAT1S   (0xB0010234)
#define PCPAT1C   (0xB0010238)

#define PCPAT0   (0xB0010240)
#define PCPAT0S   (0xB0010244)
#define PCPAT0C   (0xB0010248)

#define PCPE     (0xB0010270)
#define PCPES     (0xB0010274)
#define PCPEC     (0xB0010278)

#define PDINT    (0xB0010310)
#define PDINTS    (0xB0010314)
#define PDINTC    (0xB0010318)

#define PDMASK	  (0xB0010320)
#define PDMASKS	  (0xB0010324)
#define PDMASKC	  (0xB0010328)

#define PDPAT1   (0xB0010330)
#define PDPAT1S   (0xB0010334)
#define PDPAT1C   (0xB0010338)

#define PDPAT0   (0xB0010340)
#define PDPAT0S   (0xB0010344)
#define PDPAT0C   (0xB0010348)

#define CSPLY_VALID_HIGH (mode->csply_valid_high)
#define WRPLY_ACTIVE_HIGH (mode->wrply_active_high)
#define RDPLY_ACTIVE_HIGH (mode->rdply_active_high)
#define RSPLY_CMD_HIGH (mode->rsply_cmd_high)
#define SLCD_BUS_WIDTH (mode->bus_width)

#define GPIO_SLCD_RD (mode->rd)
#define GPIO_SLCD_CS (mode->cs)

#ifdef CONFIG_M200
#define SLCD_WR (1 << 25)
#define SLCD_RS (1 << 26)
#else
#define SLCD_WR (1 << 19)
#define SLCD_RS (1 << 18)
#endif

#define set_wr(x) outl(SLCD_WR, (x) ? PCPAT0S : PCPAT0C)
#define set_rs(x) outl(SLCD_RS, (x) ? PCPAT0S : PCPAT0C)
#define set_cs(x) gpio_set_value(GPIO_SLCD_CS, x)
#define set_rd(x) gpio_set_value(GPIO_SLCD_RD, x)

#define set_wr_low() set_wr(WRPLY_ACTIVE_HIGH)
#define set_wr_high() set_wr(!WRPLY_ACTIVE_HIGH)

#define set_rs_low() set_rs(RSPLY_CMD_HIGH)
#define set_rs_high() set_rs(!RSPLY_CMD_HIGH)

#define set_rd_low() set_rd(RDPLY_ACTIVE_HIGH)
#define set_rd_high() set_rd(!RDPLY_ACTIVE_HIGH)

#define set_cs_low() set_cs(CSPLY_VALID_HIGH)
#define set_cs_high() set_cs(!CSPLY_VALID_HIGH)

#ifndef main_clk_bypass_to_pd15
#define main_clk_bypass_to_pd15()           \
	do {                                    \
		outl(1 << 15, PDINTC);              \
		outl(1 << 15, PDMASKC);             \
		outl(1 << 15, PDPAT1C);             \
		outl(1 << 15, PDPAT0C);             \
	} while (0)
#endif

#define pd15_output_low()            \
	do {                             \
		outl(1 << 15, PDINTC);       \
		outl(1 << 15, PDMASKS);      \
		outl(1 << 15, PDPAT1C);      \
		outl(1 << 15, PDPAT0C);      \
	} while (0)

// RS, CS, D0-D7
#define __slcd_bits_pin_as_input_nopull(x)    \
	do {                                      \
		outl((x), PCINTC);                    \
		outl((x), PCMASKS);                   \
		outl((x), PCPAT1S);                   \
		outl((x), PCPAT0C);                   \
		outl((x), PCPES);                     \
	} while (0)

//RS, CS, D0-D15
#define __slcd_bits_pin_as_output_low(x)     \
	do {                                     \
		outl((x), PCINTC);                   \
		outl((x), PCMASKS);                  \
		outl((x), PCPAT1C);                  \
		outl((x), PCPAT0C);                  \
		outl((x), PCPES);                    \
	} while (0)

#define __slcd_bits_data_as_output(x)        \
	do {                                     \
 		outl((x), PCINTC);                   \
		outl((x), PCMASKS);                  \
		outl((x), PCPAT1C);                  \
		outl((x), PCPAT0S);                  \
	} while (0)

#define __slcd_bits_data_as_input(x)        \
	do {                                    \
		outl((x), PCINTC);                  \
		outl((x), PCMASKS);                 \
		outl((x), PCPAT1S);                 \
		outl((x), PCPAT0C);                 \
	} while (0)

#ifdef CONFIG_M200
#define __slcd_bits_as_function(x)          \
	do {                                    \
		set_rs_high();                      \
		outl((x), PCINTC);                  \
		outl((x), PCMASKC);                 \
		outl((x), PCPAT1S);                 \
		outl((x), PCPAT0C);                 \
	} while (0)
#else
#define __slcd_bits_as_function(x)          \
	do {                                    \
		set_rs_high();                      \
		outl((x), PCINTC);                  \
		outl((x), PCMASKC);                 \
		outl((x), PCPAT1C);                 \
		outl((x), PCPAT0C);                 \
	} while (0)
#endif
#ifdef CONFIG_M200
/**
 * jz4775 , jz4780 smart lcd mcu data pins:
 *
 * D/C: PC26
 * WR: PC25
 * RD: null
 * PCLK:PC8
 * CS: null
 * DATA:
 * 	8 bits: PC2-PC9
 * 	16 bits: PC2-PC9 PC12-PC19
 * 	18 bits: unknown from current datasheet
 *	24 bits: unknown from current datasheet
 *
 */
#define __SLCD_8_BITS 0x3fc
#define __SLCD_16_BITS 0xff3fc
#define __SLCD_18_BITS 0xfc3f0fc
#define __SLCD_24_BITS 0xff3fcff

#else
/**
 * jz4775 , jz4780 smart lcd mcu data pins:
 *
 * D/C: PC
 * WR: PC19
 * RD: null
 * PCLK:PC8
 * CS: null
 * DATA:
 * 	8 bits: PC2-PC7 PC12-PC13
 * 	16 bits: PC2-PC7 PC12-PC17 PC22-PC25
 * 	18 bits: PC2-PC7 PC12-PC17 PC22-PC27
 *	24 bits: PC0-PC7 PC10-PC17 PC20-PC27
 *
 */
#define __SLCD_8_BITS 0x30fc
#define __SLCD_16_BITS 0x3c3f0fc
#define __SLCD_18_BITS 0xfc3f0fc
#define __SLCD_24_BITS 0xff3fcff
#endif

#define __SLCD_WR_RS_BITS (SLCD_WR | SLCD_RS)

#define __slcd_8bit_data_as_output() __slcd_bits_data_as_output(__SLCD_8_BITS)
#define __slcd_16bit_data_as_output() __slcd_bits_data_as_output(__SLCD_16_BITS)
#define __slcd_18bit_data_as_output() __slcd_bits_data_as_output(__SLCD_18_BITS)
#define __slcd_24bit_data_as_output() __slcd_bits_data_as_output(__SLCD_24_BITS)

#define __slcd_8bit_data_as_input() __slcd_bits_data_as_input(__SLCD_8_BITS)
#define __slcd_16bit_data_as_input() __slcd_bits_data_as_input(__SLCD_16_BITS)
#define __slcd_18bit_data_as_input() __slcd_bits_data_as_input(__SLCD_18_BITS)
#define __slcd_24bit_data_as_input() __slcd_bits_data_as_input(__SLCD_24_BITS)

#define __slcd_8bit_as_function() __slcd_bits_as_function(__SLCD_8_BITS | __SLCD_WR_RS_BITS)
#define __slcd_16bit_as_function() __slcd_bits_as_function(__SLCD_16_BITS | __SLCD_WR_RS_BITS)
#define __slcd_18bit_as_function() __slcd_bits_as_function(__SLCD_18_BITS | __SLCD_WR_RS_BITS)
#define __slcd_24bit_as_function() __slcd_bits_as_function(__SLCD_24_BITS | __SLCD_WR_RS_BITS)

#define __slcd_8bit_pin_as_input_nopull() __slcd_bits_pin_as_input_nopull(__SLCD_8_BITS | __SLCD_WR_RS_BITS)
#define __slcd_16bit_pin_as_input_nopull() __slcd_bits_pin_as_input_nopull(__SLCD_16_BITS | __SLCD_WR_RS_BITS)
#define __slcd_18bit_pin_as_input_nopull() __slcd_bits_pin_as_input_nopull(__SLCD_18_BITS | __SLCD_WR_RS_BITS)
#define __slcd_24bit_pin_as_input_nopull() __slcd_bits_pin_as_input_nopull(__SLCD_24_BITS | __SLCD_WR_RS_BITS)

#define __slcd_8bit_pin_as_output_low() __slcd_bits_pin_as_output_low(__SLCD_8_BITS | __SLCD_WR_RS_BITS)
#define __slcd_16bit_pin_as_output_low() __slcd_bits_pin_as_output_low(__SLCD_16_BITS | __SLCD_WR_RS_BITS)
#define __slcd_18bit_pin_as_output_low() __slcd_bits_pin_as_output_low(__SLCD_18_BITS | __SLCD_WR_RS_BITS)
#define __slcd_24bit_pin_as_output_low() __slcd_bits_pin_as_output_low(__SLCD_24_BITS | __SLCD_WR_RS_BITS)

#define __slcd_wr_rs_as_output()					\
	do {											\
		outl(__SLCD_WR_RS_BITS, PCINTC);					\
		outl(__SLCD_WR_RS_BITS, PCMASKS);					\
		outl(__SLCD_WR_RS_BITS, PCPAT1C);					\
		outl(__SLCD_WR_RS_BITS, PCPAT0S);					\
	} while (0)

#ifdef CONFIG_M200
#define __write_data_8(data)                                  \
	do {                                                      \
		unsigned int low_8bits = data & 0xff;                 \
		outl((low_8bits << 2), PCPAT0S);                      \
		low_8bits = ~data & 0xff;                             \
		outl((low_8bits << 2),PCPAT0C);                       \
	} while (0)

#define __write_data_16(data)                                                        \
	do {                                                                             \
		unsigned int low_8bits = data & 0xff;                                        \
		unsigned int high_8bits = (data >> 8) && 0xff;                               \
		outl((high_8bits << 12) | (low_8bits << 2), PCPAT0S);                        \
		low_8bits = ~data & 0xff;                                                    \
		high_8bits = (~data >> 8) & 0xff;                                            \
		outl((high_8bits << 12) | (low_8bits << 2), PCPAT0C);                        \
	} while (0)

#define __write_data_18(data)                                                        \
	do {                                                                             \
		unsigned int low_6bits = data & 0x3f;                                        \
		unsigned int middle_6bits = (data >> 6) && 0x3f;                             \
		unsigned int high_6bits = (data >> 12) && 0x3f;                              \
		outl((high_6bits << 22) | (middle_6bits << 12) | (low_6bits << 2), PCPAT0S); \
		low_6bits = ~data & 0x3f;                                                    \
		middle_6bits = (~data >> 6) & 0x3f;                                          \
		high_6bits = (~data >> 12) & 0x3f;                                           \
		outl((high_6bits << 22) | (middle_6bits << 12) | (low_6bits << 2), PCPAT0C); \
	} while (0)

#define __write_data_24(data)                                                        \
	do {                                                                             \
		unsigned int low_8bits = data & 0xff;                                        \
		unsigned int middle_8bits = (data >> 8) && 0xff;                             \
		unsigned int high_8bits = (data >> 16) && 0xff;                              \
		outl((high_8bits << 20) | (middle_8bits << 10) | (low_8bits << 0), PCPAT0S); \
		low_8bits = ~data & 0xff;                                                    \
		middle_8bits = (~data >> 8) & 0xff;                                          \
		high_8bits = (~data >> 16) & 0xff;                                           \
		outl((high_8bits << 20) | (middle_8bits << 10) | (low_8bits << 0), PCPAT0C); \
	} while (0)

#define __read_data_8()                                   \
	({                                                    \
		unsigned int data = inl(PCPIN);                   \
		unsigned int low_8bits = (data >> 2) & 0xff;      \
		low_8bits;                                        \
	})

#define __read_data_16()                                             \
	({                                                               \
		unsigned int data = inl(PCPIN);                              \
		unsigned int low_8bits = (data >> 2) & 0xff;                 \
		unsigned char high_8bits = (data >> 12) & 0xff;              \
		data = (high_8bits << 8)  | low_8bits;                       \
		data;                                                        \
	})

#define __read_data_18()                                             \
	({                                                               \
		unsigned int data = inl(PCPIN);                              \
		unsigned int low_6bits = (data >> 2) & 0x3f;                 \
		unsigned int middle_6bits = (data >> 12) & 0x3f;             \
		unsigned char high_6bits = (data >> 22) & 0x3f;              \
		data = (high_6bits << 12) | (middle_6bits << 6) | low_6bits; \
		data;                                                        \
	})

#define __read_data_24()                                             \
	({                                                               \
		unsigned int data = inl(PCPIN);                              \
		unsigned int low_8bits = (data >> 0) & 0xff;                 \
		unsigned int middle_8bits = (data >> 10) & 0xff;             \
		unsigned char high_8bits = (data >> 20) & 0xff;              \
		data = (high_8bits << 16) | (middle_8bits << 8) | low_8bits; \
		data;                                                        \
	})

#else
#define __write_data_8(data)                                  \
	do {                                                      \
		unsigned char low_6bits = data & 0x3f;                \
		unsigned char high_2bits = (data >> 6) & 0x03;        \
		outl((high_2bits << 12) | (low_6bits << 2), PCPAT0S); \
		low_6bits = ~data & 0x3f;                             \
		high_2bits = (~data >> 6) & 0x03;                     \
		outl((high_2bits << 12) | (low_6bits << 2),PCPAT0C);  \
	} while (0)

#define __write_data_16(data)                                                        \
	do {                                                                             \
		unsigned int low_6bits = data & 0x3f;                                        \
		unsigned int middle_6bits = (data >> 6) && 0x3f;                             \
		unsigned int high_4bits = (data >> 12) && 0x0f;                              \
		outl((high_4bits << 22) | (middle_6bits << 12) | (low_6bits << 2), PCPAT0S); \
		low_6bits = ~data & 0x3f;                                                    \
		middle_6bits = (~data >> 6) & 0x3f;                                          \
		high_4bits = (~data >> 12) & 0x0f;                                           \
		outl((high_4bits << 22) | (middle_6bits << 12) | (low_6bits << 2), PCPAT0C); \
	} while (0)

#define __write_data_18(data)                                                        \
	do {                                                                             \
		unsigned int low_6bits = data & 0x3f;                                        \
		unsigned int middle_6bits = (data >> 6) && 0x3f;                             \
		unsigned int high_6bits = (data >> 12) && 0x3f;                              \
		outl((high_6bits << 22) | (middle_6bits << 12) | (low_6bits << 2), PCPAT0S); \
		low_6bits = ~data & 0x3f;                                                    \
		middle_6bits = (~data >> 6) & 0x3f;                                          \
		high_6bits = (~data >> 12) & 0x3f;                                           \
		outl((high_6bits << 22) | (middle_6bits << 12) | (low_6bits << 2), PCPAT0C); \
	} while (0)

#define __write_data_24(data)                                                        \
	do {                                                                             \
		unsigned int low_8bits = data & 0xff;                                        \
		unsigned int middle_8bits = (data >> 8) && 0xff;                             \
		unsigned int high_8bits = (data >> 16) && 0xff;                              \
		outl((high_8bits << 20) | (middle_8bits << 10) | (low_8bits << 0), PCPAT0S); \
		low_8bits = ~data & 0xff;                                                    \
		middle_8bits = (~data >> 8) & 0xff;                                          \
		high_8bits = (~data >> 16) & 0xff;                                           \
		outl((high_8bits << 20) | (middle_8bits << 10) | (low_8bits << 0), PCPAT0C); \
	} while (0)

#define __read_data_8()                                   \
	({                                                    \
		unsigned int data = inl(PCPIN);                   \
		unsigned char low_6bits = (data >> 2) & 0x3f;     \
		unsigned char high_2bits = (data >> 12) & 0x03;   \
		data = (high_2bits << 6) | low_6bits;             \
		data;                                             \
	})

#define __read_data_16()                                             \
	({                                                               \
		unsigned int data = inl(PCPIN);                              \
		unsigned int low_6bits = (data >> 2) & 0x3f;                 \
		unsigned int middle_6bits = (data >> 12) & 0x3f;             \
		unsigned char high_4bits = (data >> 22) & 0x0f;              \
		data = (high_4bits << 12) | (middle_6bits << 6) | low_6bits; \
		data;                                                        \
	})

#define __read_data_18()                                             \
	({                                                               \
		unsigned int data = inl(PCPIN);                              \
		unsigned int low_6bits = (data >> 2) & 0x3f;                 \
		unsigned int middle_6bits = (data >> 12) & 0x3f;             \
		unsigned char high_6bits = (data >> 22) & 0x3f;              \
		data = (high_6bits << 12) | (middle_6bits << 6) | low_6bits; \
		data;                                                        \
	})

#define __read_data_24()                                             \
	({                                                               \
		unsigned int data = inl(PCPIN);                              \
		unsigned int low_8bits = (data >> 0) & 0xff;                 \
		unsigned int middle_8bits = (data >> 10) & 0xff;             \
		unsigned char high_8bits = (data >> 20) & 0xff;              \
		data = (high_8bits << 16) | (middle_8bits << 8) | low_8bits; \
		data;                                                        \
	})
#endif

/* need to do with 8, 16, 18, 24 bits */
static inline unsigned int  __read_data(struct slcd_mode *mode) {
	switch (SLCD_BUS_WIDTH) {
	case 8: return __read_data_8();
	case 16: return __read_data_16();
	case 18: return __read_data_18();
	case 24: return __read_data_24();
	default: break;
	}
	return 0;
}

static inline void __write_data(struct slcd_mode *mode, unsigned int data) {
	switch (SLCD_BUS_WIDTH) {
	case 8:  __write_data_8(data); return ;
	case 16:  __write_data_16(data); return ;
	case 18:  __write_data_18(data); return ;
	case 24:  __write_data_24(data); return ;
	default:  break;
	}
}

void slcd_data_as_output(struct slcd_mode *mode) {
	if_gpio_direction_output(GPIO_SLCD_CS, CSPLY_VALID_HIGH ? 0 : 1);
	udelay(10);
	if_gpio_direction_output(GPIO_SLCD_RD, RDPLY_ACTIVE_HIGH ? 0 : 1);

	__slcd_wr_rs_as_output();
	set_wr_high();
	set_rs_low();

	pr_info("bus width :%d\n", SLCD_BUS_WIDTH);
	switch (SLCD_BUS_WIDTH) {
	case 8: __slcd_8bit_data_as_output(); break;
	case 16: __slcd_16bit_data_as_output(); break;
	case 18:  __slcd_18bit_data_as_output(); break;
	case 24:  __slcd_24bit_data_as_output(); break;
	default:  break;
	}
	if_gpio_direction_output(GPIO_SLCD_CS, CSPLY_VALID_HIGH ? 1 : 0);
	udelay(10);
}

void slcd_pin_as_function(struct slcd_mode *mode) {
	if_gpio_direction_output(GPIO_SLCD_CS, CSPLY_VALID_HIGH ? 0 : 1);
	udelay(10);
	switch (SLCD_BUS_WIDTH) {
	case 8: __slcd_8bit_as_function(); break;
	case 16: __slcd_16bit_as_function(); break;
	case 18:  __slcd_18bit_as_function(); break;
	case 24:  __slcd_24bit_as_function(); break;
	default:  break;
	}
	if_gpio_direction_output(GPIO_SLCD_CS, CSPLY_VALID_HIGH ? 1 : 0);
	udelay(10);

}

void slcd_data_as_input(struct slcd_mode *mode) {
	if_gpio_direction_output(GPIO_SLCD_CS, CSPLY_VALID_HIGH ? 0 : 1);
	udelay(10);
	if_gpio_direction_output(GPIO_SLCD_RD, RDPLY_ACTIVE_HIGH ? 0 : 1);

	__slcd_wr_rs_as_output();
	set_wr_high();
	set_rs_low();

	pr_info("bus width :%d\n", SLCD_BUS_WIDTH);

	switch (SLCD_BUS_WIDTH) {
	case 8: __slcd_8bit_data_as_input(); break;
	case 16: __slcd_16bit_data_as_input(); break;
	case 18:  __slcd_18bit_data_as_input(); break;
	case 24:  __slcd_24bit_data_as_input(); break;
	default:  break;
	}
	if_gpio_direction_output(GPIO_SLCD_CS, CSPLY_VALID_HIGH ? 1 : 0);
	udelay(10);
}

void slcd_pin_as_input_nopull(struct slcd_mode *mode) {
	if_gpio_direction_output(GPIO_SLCD_CS, CSPLY_VALID_HIGH ? 0 : 1);
	udelay(10);

	switch (SLCD_BUS_WIDTH) {
	case 8: __slcd_8bit_pin_as_input_nopull(); break;
	case 16: __slcd_16bit_pin_as_input_nopull(); break;
	case 18:  __slcd_18bit_pin_as_input_nopull(); break;
	case 24:  __slcd_24bit_pin_as_input_nopull(); break;
	default:  break;
	}
	if_gpio_direction_output(GPIO_SLCD_CS, CSPLY_VALID_HIGH ? 1 : 0);
	udelay(10);
}

void slcd_pin_as_output_low(struct slcd_mode *mode) {
	if_gpio_direction_output(GPIO_SLCD_CS, CSPLY_VALID_HIGH ? 0 : 1);
	udelay(10);

	switch (SLCD_BUS_WIDTH) {
	case 8: __slcd_8bit_pin_as_output_low(); break;
	case 16: __slcd_16bit_pin_as_output_low(); break;
	case 18:  __slcd_18bit_pin_as_output_low(); break;
	case 24:  __slcd_24bit_pin_as_output_low(); break;
	default:  break;
	}
	if_gpio_direction_output(GPIO_SLCD_CS, CSPLY_VALID_HIGH ? 1 : 0);
	udelay(10);
}

/* ------------------------------------ */

static void little_delay(int count) {
#if 0
	count = count / 10;
	while (count--);
#endif
}

static inline void slcd_write_data_internal(struct slcd_mode *mode, unsigned int data) {
	set_rs_high();
	little_delay(2);
	__write_data(mode,data);
	little_delay(2);
	set_wr_low();
	little_delay(5);
	set_wr_high();
	little_delay(5);
	set_rs_low();
	little_delay(2);
}

static inline void slcd_write_cmd_internal(struct slcd_mode *mode, unsigned int cmd) {
	set_rs_low();
	little_delay(2);
	__write_data(mode,cmd);
	little_delay(2);
	set_wr_low();
	little_delay(5);
	set_wr_high();
	little_delay(5);
	set_rs_high();
	little_delay(2);
}

void slcd_write_cmd(struct slcd_mode *mode, unsigned int cmd) {
	slcd_write_cmd_internal(mode, cmd);
}

void slcd_write_data(struct slcd_mode *mode, unsigned int data) {
	slcd_write_data_internal(mode, data);
}

int slcd_read_data(struct slcd_mode *mode, unsigned int *data, int num) {
	int i;

	slcd_data_as_input(mode);
	for (i=0; i<num; i++) {
		set_rd_low();
		udelay(1);
		data[i] = (unsigned int)__read_data(mode);
		set_rd_high();
		udelay(1);
	}
	slcd_data_as_output(mode);

	return 0;
}

void slcd_write_buffer_16_twice(struct slcd_mode *mode, struct fb_struct *fbs, void *base) {
	unsigned int i, j;
	u16 *ptr = (u16*)base;
	unsigned int k = fbs->pixels_per_line - fbs->xres;

	for (i = 0; i < fbs->yres; ++i) {
		for (j = 0; j < fbs->xres; ++j) {
			slcd_write_data_internal(mode, *ptr >> 8);
			slcd_write_data_internal(mode, *ptr );
			++ptr;
		}
		ptr += k;
	}
}

void slcd_write_buffer_18_twice(struct slcd_mode *mode, struct fb_struct *fbs, void *base) {
	unsigned int i, j;
	u32 *ptr = (u32*)base;
	unsigned int k = fbs->pixels_per_line - fbs->xres;

	for (i = 0; i < fbs->yres; ++i) {
		for (j = 0; j < fbs->xres; ++j) {
			unsigned int r = *ptr & (0x1f << 19);
			unsigned int g = *ptr & (0x3f << 10);
			unsigned int b = *ptr & (0x1f << 3);
			slcd_write_data_internal(mode, r >> 16 | g >> 13);
			slcd_write_data_internal(mode, g >> 5 | b >> 3);
			++ptr;
		}
		ptr += k;
	}
}

void slcd_write_buffer_18_thrice(struct slcd_mode *mode, struct fb_struct *fbs, void *base) {
	unsigned int i, j;
	u32 *ptr = (u32*)base;
	unsigned int k = fbs->pixels_per_line - fbs->xres;

	for (i = 0; i < fbs->yres; ++i) {
		for (j = 0; j < fbs->xres; ++j) {
			slcd_write_data_internal(mode, *ptr >> 16);
			slcd_write_data_internal(mode, *ptr >> 8);
			slcd_write_data_internal(mode, *ptr);
			++ptr;
		}
		ptr += k;
	}
}

void slcd_write_buffer_24_twice(struct slcd_mode *mode, struct fb_struct *fbs, void *base) {
	unsigned int i, j;
	u32 *ptr = (u32*)base;
	unsigned int k = fbs->pixels_per_line - fbs->xres;

	for (i = 0; i < fbs->yres; ++i) {
		for (j = 0; j < fbs->xres; ++j) {
			unsigned int r = *ptr & (0x1f << 19);
			unsigned int g = *ptr & (0x3f << 10);
			unsigned int b = *ptr & (0x1f << 3);
			slcd_write_data_internal(mode, r >> 16 | g >> 13);
			slcd_write_data_internal(mode, g >> 5 | b >> 3);
			++ptr;
		}
		ptr += k;
	}
}

void slcd_write_buffer_24_thrice(struct slcd_mode *mode, struct fb_struct *fbs, void *base) {
	unsigned int i, j;
	u32 *ptr = (u32*)base;
	unsigned int k = fbs->pixels_per_line - fbs->xres;

	for (i = 0; i < fbs->yres; ++i) {
		for (j = 0; j < fbs->xres; ++j) {
			slcd_write_data_internal(mode, *ptr >> 16);
			slcd_write_data_internal(mode, *ptr >> 8);
			slcd_write_data_internal(mode, *ptr);
			++ptr;
		}
		ptr += k;
	}
}

int slcd_request_gpio(struct slcd_mode *mode) {
	int ret;

	pr_info("lcd cs : %d\n", GPIO_SLCD_CS);
	pr_info("lcd RD : %d\n", GPIO_SLCD_RD);

	ret = if_gpio_request(GPIO_SLCD_CS, "slcd-cs");
	if (ret) {
		pr_info("Failed to request slcd cs\n");
		ret = -EINVAL;
		goto error_gpio_request_cs_failed;
	}

	ret = if_gpio_request(GPIO_SLCD_RD, "slcd-cs");
	if (ret) {
		pr_info("Failed to request slcd rd\n");
		ret = -EINVAL;
		goto error_gpio_request_rd_failed;
	}
	return 0;

error_gpio_request_rd_failed:
	if_gpio_free(GPIO_SLCD_CS);
error_gpio_request_cs_failed:
	return ret;
}

void slcd_free_gpio(struct slcd_mode *mode) {
	if_gpio_free(GPIO_SLCD_CS);
	if_gpio_free(GPIO_SLCD_RD);
}

void slcd_set_cs_low(struct slcd_mode *mode) {
	if_gpio_direction_output(GPIO_SLCD_CS, CSPLY_VALID_HIGH ? 1 : 0);
}

void slcd_set_cs_high(struct slcd_mode *mode) {
	if_gpio_direction_output(GPIO_SLCD_CS, CSPLY_VALID_HIGH ? 0 : 1);
}

void slcd_set_rd_low(struct slcd_mode *mode) {
	if_gpio_direction_output(GPIO_SLCD_CS, CSPLY_VALID_HIGH ? 1 : 0);
}

void slcd_set_rd_high(struct slcd_mode *mode) {
	if_gpio_direction_output(GPIO_SLCD_CS, CSPLY_VALID_HIGH ? 0 : 1);
}

int slcd_init_interface(struct slcd_mode *mode) {

	return 0;
}

void slcd_write_datas(struct slcd_mode *mode, struct smart_lcd_data_table *data_table, unsigned int length_data_table) {
	unsigned int i;

	slcd_data_as_output(mode);

	for (i=0; i < length_data_table; i++) {
		if (data_table[i].type == 1)
			slcd_write_cmd(mode, data_table[i].value);
		else if (data_table[i].type == 2)
			slcd_write_data(mode, data_table[i].value);
		if (data_table[i].udelay)
			udelay(data_table[i].udelay);
	}

	slcd_pin_as_function(mode);
}
