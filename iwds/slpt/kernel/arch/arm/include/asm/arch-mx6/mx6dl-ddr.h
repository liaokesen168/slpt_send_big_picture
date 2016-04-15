/*
 * Copyright (C) 2013 Boundary Devices Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef __ASM_ARCH_MX6DLS_DDR_H__
#define __ASM_ARCH_MX6DLS_DDR_H__

#ifndef CONFIG_MX6DL
#ifndef CONFIG_MX6S
#error "wrong CPU"
#endif
#endif

#define MX6_IOM_DRAM_DQM0	0x020e0470
#define MX6_IOM_DRAM_DQM1	0x020e0474
#define MX6_IOM_DRAM_DQM2	0x020e0478
#define MX6_IOM_DRAM_DQM3	0x020e047c
#define MX6_IOM_DRAM_DQM4	0x020e0480
#define MX6_IOM_DRAM_DQM5	0x020e0484
#define MX6_IOM_DRAM_DQM6	0x020e0488
#define MX6_IOM_DRAM_DQM7	0x020e048c

#define MX6_IOM_DRAM_CAS	0x020e0464
#define MX6_IOM_DRAM_RAS	0x020e0490
#define MX6_IOM_DRAM_RESET	0x020e0494
#define MX6_IOM_DRAM_SDCLK_0	0x020e04ac
#define MX6_IOM_DRAM_SDCLK_1	0x020e04b0
#define MX6_IOM_DRAM_SDBA2	0x020e04a0
#define MX6_IOM_DRAM_SDCKE0	0x020e04a4
#define MX6_IOM_DRAM_SDCKE1	0x020e04a8
#define MX6_IOM_DRAM_SDODT0	0x020e04b4
#define MX6_IOM_DRAM_SDODT1	0x020e04b8

#define MX6_IOM_DRAM_SDQS0	0x020e04bc
#define MX6_IOM_DRAM_SDQS1	0x020e04c0
#define MX6_IOM_DRAM_SDQS2	0x020e04c4
#define MX6_IOM_DRAM_SDQS3	0x020e04c8
#define MX6_IOM_DRAM_SDQS4	0x020e04cc
#define MX6_IOM_DRAM_SDQS5	0x020e04d0
#define MX6_IOM_DRAM_SDQS6	0x020e04d4
#define MX6_IOM_DRAM_SDQS7	0x020e04d8

#define MX6_IOM_GRP_B0DS	0x020e0764
#define MX6_IOM_GRP_B1DS	0x020e0770
#define MX6_IOM_GRP_B2DS	0x020e0778
#define MX6_IOM_GRP_B3DS	0x020e077c
#define MX6_IOM_GRP_B4DS	0x020e0780
#define MX6_IOM_GRP_B5DS	0x020e0784
#define MX6_IOM_GRP_B6DS	0x020e078c
#define MX6_IOM_GRP_B7DS	0x020e0748
#define MX6_IOM_GRP_ADDDS	0x020e074c
#define MX6_IOM_DDRMODE_CTL	0x020e0750
#define MX6_IOM_GRP_DDRPKE	0x020e0754
#define MX6_IOM_GRP_DDRMODE	0x020e0760
#define MX6_IOM_GRP_CTLDS	0x020e076c
#define MX6_IOM_GRP_DDR_TYPE	0x020e0774

#endif	/*__ASM_ARCH_MX6S_DDR_H__ */
