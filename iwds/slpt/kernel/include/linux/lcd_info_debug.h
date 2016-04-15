#ifndef _LCD_INFO_DEBUG_H_
#define _LCD_INFO_DEBUG_H_

#define lcd_info(x) printf("%s", x)

#define lcd_info_tag(x...) printf("%s:%s", x)

#define lcd_info_dec(x, dec)                    \
	do {                                        \
		printf (x);								\
		printf("%d\n", dec);					\
	} while (0)

#define lcd_info_hex(x, hex)                    \
	do {                                        \
		printf(x);								\
		printf("%x\n", hex);					\
	} while (0)

#define lcd_info_dump_dec(dec) printf("%d:\n", dec)

#define lcd_info_dump_hex(hex) printf("%d:\n", hex)

#endif /* _LCD_INFO_DEBUG_H_ */
