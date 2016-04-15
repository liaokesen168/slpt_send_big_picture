#
# Copyright (C) 2014 Ingenic Semiconductor Inc.
#
# Makefile for the JZ4775 slpt code.
#

CROSS := mips-linux-gnu-
CFLAGS := -O2 -mips32 -fno-pic -mno-abicalls -EL -g -Wall -msoft-float -Bstatic
LDFLAGS := -nostdlib -T target.ld -EL

OBJS := head.o main.o slpt_api.o

all: slpt.bin
#	cat mbr fw_ddr1.bin > all.bin

slpt.bin: slpt.elf
	$(CROSS)objcopy -O binary $< $@
	$(CROSS)objdump -D $< > fw.dump
	$(CROSS)objdump -h $< > fw.map
	$(CROSS)nm -n $< > System.map

slpt.elf: $(OBJS)
	$(CROSS)ld $(LDFLAGS) $(OBJS) -o $@

.c.o:
	$(CROSS)gcc $(CFLAGS) -c $< -o $@
.S.o:
	$(CROSS)gcc -D__ASSEMBLY__ $(CFLAGS) -c $< -o $@
clean:
	rm -fr *.o *.elf *.bin *.dump *.map
