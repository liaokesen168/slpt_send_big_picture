#
# Copyright (C) 2013 Ingenic Semiconductor Co., Ltd.
# Authors: Kage Shen <kkshen@ingenic.cn>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version
# 3 of the License, or (at your option) any later version.
#

PLATFORM_CPPFLAGS += -march=mips32
PLATFORM_CPPFLAGS += -I$(TOPDIR)/

#CONFIG_STANDALONE_LOAD_ADDR ?= 0x80120000 -T mips.lds
