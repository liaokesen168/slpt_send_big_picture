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
PLATFORM_CPPFLAGS += -mabi=32 -DCONFIG_32BIT
PLATFORM_CPPFLAGS += -DCONFIG_SOC_NAME=\"$(SOC)\"

ifdef CONFIG_SYS_BIG_ENDIAN
PLATFORM_LDFLAGS  += -m elf32btsmip
else
PLATFORM_LDFLAGS  += -m elf32ltsmip
endif

USE_PRIVATE_LIBGCC = yes

CONFIG_STANDALONE_LOAD_ADDR ?= 0x80200000 -T mips.lds
