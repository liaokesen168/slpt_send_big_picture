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

START = start.o

LIB	= $(obj)lib$(SOC).o

COBJS-y = jz4775.o cpm.o pfsv1_power.o pfsv1_misc.o

COBJS-$(CONFIG_PFSV1) += pfsv1_nand.o

SRCS := $(START:.o=.S) $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS := $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))
START := $(addprefix $(obj),$(START))

all: $(obj).depend $(START) $(LIB)

$(LIB): $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
