#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libingenic_display.o

COBJS-$(CONFIG_JZ_FB) += jzfb.o lcd_panel_data.o lcd_mcu.o lcd_board_info_manager.o \
	lcd_draw.o lcd_area_ops.o bmp_gen.o lcd_img_display.o lcd_color_rotate.o sin_cos_datas.o \
	lcd_region_rotate.o \
	color_map.o color_map_save.o \
	view/picture.o view/view.o view/view_utils.o view/view_grp.o \
	view/pic_view.o view/num_view.o view/flash_pic_view.o view/text_view.o \
	view/rotate.o  view/rotate_pic.o \
	view/picture_header.o \
	clock/digital_clock.o \
	clock/analog_clock.o \
	clock/analog_base_clock.o \
	clock/analog_week_clock.o \
	clock/analog_month_clock.o \
	clock/analog_second_clock.o \
	clock/analog_minute_clock.o \
	clock/analog_hour_clock.o \
	clock/date_en_view.o \
	clock/date_cn_view.o \
	clock/week_en_view.o \
	clock/week_cn_view.o \
	clock/year_en_view.o \
	clock/time_view.o \
	clock/current_time.o clock/time_notify.o \
	clock/background.o

COBJS-$(CONFIG_JZ_FB) += \
	utils/arg_parse.o \
	utils/malloc_with_name.o \
	utils/string.o \
	./utils/item_parser.o \
	./utils/key_reader.o \
	./utils/key_writer.o

COBJS-$(CONFIG_JZ_FB) += \
	sview/core/sview.o \
	sview/core/sview_methods.o \
	sview/core/sview_grp.o \
	sview/core/background.o \
	sview/core/pic_sview.o \
	sview/core/num_sview.o \
	sview/core/linear_layout.o \
	sview/core/absolute_layout.o \
	sview/core/frame_layout.o \
	sview/core/root_sview.o

COBJS-$(CONFIG_JZ_FB) += \
	sview/time/time_num_sview.o \
	sview/time/secondL_sview.o \
	sview/time/secondH_sview.o \
	sview/time/minuteL_sview.o \
	sview/time/minuteH_sview.o \
	sview/time/hourL_sview.o \
	sview/time/hourH_sview.o \
	sview/time/dayL_sview.o \
	sview/time/dayH_sview.o \
	sview/time/week_sview.o \
	sview/time/monthL_sview.o \
	sview/time/monthH_sview.o \
	sview/time/year0_sview.o \
	sview/time/year1_sview.o \
	sview/time/year2_sview.o \
	sview/time/year3_sview.o \
	sview/analog/rotate2.o \
	sview/analog/rotate_pic_sview.o \
	sview/analog/low_voltage_sview.o \
	sview/analog/time/analog_time_sview.o \
	sview/analog/time/analog_second_sview.o \
	sview/analog/time/analog_minute_sview.o \
	sview/analog/time/analog_hour_sview.o \
	sview/analog/time/analog_day_sview.o \
	sview/analog/time/analog_week_sview.o \
	sview/analog/time/analog_month_sview.o \
	sview/analog/time/analog_am_pm_sview.o \
	sview/analog/time/analog_hour_with_minute_sview.o

COBJS-$(CONFIG_JZ_FB) += \
	sview/utils/sview_type_to_string.o \
	sview/utils/create_sview_from_key_reader.o

COBJS-$(CONFIG_M200) += lcd_clk.o
COBJS-$(CONFIG_JZ_MIPI_DSI) += jz_mipi_dsi/jz_mipi_dsih_hal.o \
	jz_mipi_dsi/jz_mipi_dsi_lowlevel.o \
	jz_mipi_dsi/write_command_to_mipi.o

COBJS-$(CONFIG_SLCD_SEPS645B) += panels/seps645b_data.o panels/seps645b_oled.o
COBJS-$(CONFIG_SLCD_TRULY240240) += panels/truly240240_data.o panels/truly240240_slcd.o
COBJS-$(CONFIG_LCD_X163) += panels/auo_x163_data.o panels/auo_x163_oled.o
COBJS-$(CONFIG_SLCD_TRULY320320) += panels/truly320320_data.o panels/truly320320_slcd.o
COBJS-$(CONFIG_LCD_BOE_TFT320320) += panels/boe_tft320320_data.o panels/boe_tft320320_lcd.o
COBJS-$(CONFIG_LCD_AUO_H139BLN01) += panels/auo_h139bln01_data.o panels/auo_h139bln01_lcd.o
COBJS-$(CONFIG_LCD_EDO_E1392AM1) += panels/edo_e1392am1_data.o panels/edo_e1392am1_lcd.o
COBJS-$(CONFIG_LCD_H160_TFT320320) += panels/h160_tft320320_data.o panels/h160_tft320320_lcd.o
COBJS-$(CONFIG_LCD_ARS_NT35350) += panels/ars_nt35350_data.o panels/ars_nt35350_lcd.o

COBJS	:= $(sort $(COBJS-y))
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
