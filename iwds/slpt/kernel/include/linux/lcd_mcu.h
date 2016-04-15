#ifndef _LCD_MCU_H_
#define _LCD_MCU_H_

struct slcd_mode {
	int rd;						/* mcu rd pin */
	int cs;						/* mcu cs pin */
	unsigned int bus_width;	/* 8, 16, 18, 24 */
	unsigned int csply_valid_high:1;
	unsigned int rsply_cmd_high:1;
	unsigned int wrply_active_high:1; /* WR initial level will be different from WR Polarity */
	unsigned int rdply_active_high:1; /* RD initial level will be different from RD Polarity */

	struct {
		unsigned int mode;		/* gpio, lcd function, output low, inputpull */
	} s;
};

struct fb_struct;
struct smart_lcd_data_table;

extern void slcd_set_cs_low(struct slcd_mode *mode);
extern void slcd_set_cs_high(struct slcd_mode *mode);

void slcd_set_rd_low(struct slcd_mode *mode);
void slcd_set_rd_high(struct slcd_mode *mode);

extern void slcd_data_as_output(struct slcd_mode *mode);
void slcd_pin_as_function(struct slcd_mode *mode);
extern void slcd_data_as_input(struct slcd_mode *mode);

extern void slcd_pin_as_input_nopull(struct slcd_mode *mode);
extern void slcd_pin_as_output_low(struct slcd_mode *mode);

extern void slcd_write_cmd(struct slcd_mode *mode, unsigned int cmd);
extern void slcd_write_data(struct slcd_mode *mode, unsigned int data);
extern int slcd_read_data(struct slcd_mode *mode, unsigned int *data, int num);

void slcd_write_buffer_16_twice(struct slcd_mode *mode, struct fb_struct *fbs, void *base);
void slcd_write_buffer_18_twice(struct slcd_mode *mode, struct fb_struct *fbs, void *base);
void slcd_write_buffer_18_thrice(struct slcd_mode *mode, struct fb_struct *fbs, void *base);
void slcd_write_buffer_24_twice(struct slcd_mode *mode, struct fb_struct *fbs, void *base);
void slcd_write_buffer_24_thrice(struct slcd_mode *mode, struct fb_struct *fbs, void *base);

extern int slcd_request_gpio(struct slcd_mode *mode);
extern void slcd_free_gpio(struct slcd_mode *mode);
extern int slcd_init_interface(struct slcd_mode *mode);

extern void slcd_write_datas(struct slcd_mode *mode, struct smart_lcd_data_table *data_table, unsigned int length_data_table);
#endif /* _LCD_MCU_H_ */
