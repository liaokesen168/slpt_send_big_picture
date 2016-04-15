#
# Copyright (C) 2013 Ingenic Semiconductor Co., Ltd.
# Authors: ZhangYanMing <ymzhang@ingenic.cn>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version
# 3 of the License, or (at your option) any later version.
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(CPU)-common.o

SOBJS-y = slpt_app_test_adc.o slpt_match_voltage.o slpt_app_low_voltage_detect.o

COBJS-y += adc.o gpemc.o bch.o cpu.o timer.o uart.o board.o init_sequence.o \
			cmd_platforminfo.o suspend.o default_battery_cv.o slpt_gpio.o

COBJS-$(CONFIG_JZ47XX_SLPT) += cmd_slpt.o slpt_irq_handler.o
COBJS-$(CONFIG_SLPT_CLOCK) += slpt_app_clock.o font_library.o  \

COBJS-$(CONFIG_VIEW) += view/display.o view/analog_clock.o view/digital_clock.o \
	view/date_en_view.o view/date_cn_view.o \
	view/week_en_view.o view/week_cn_view.o view/time_view.o view/low_battery.o view/charge_picture.o \
	view/analog_week_clock.o view/analog_month_clock.o view/analog_second_clock.o view/analog_minute_clock.o \
	view/analog_hour_clock.o view/year_en_view.o

COBJS-$(CONFIG_SVIEW) += sview/display.o

COBJS-$(CONFIG_SLPT_POWERKEY) += slpt_app_powerkey.o
COBJS-$(CONFIG_FRIZZ)     += frizz.o
COBJS-$(CONFIG_SLPT_ALARM) += slpt_app_alarm.o
COBJS-$(CONFIG_SLPT_TIMER) += slpt_app_timer.o
COBJS-$(CONFIG_SLPT_BATTERY) += slpt_app_battery.o slpt_get_capacity.o

SRCS := $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS := $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))

all: $(obj).depend $(LIB)

$(LIB): $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
