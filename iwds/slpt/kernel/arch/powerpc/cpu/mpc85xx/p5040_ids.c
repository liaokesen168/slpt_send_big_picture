/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/fsl_portals.h>
#include <asm/fsl_liodn.h>

#ifdef CONFIG_SYS_DPAA_QBMAN
struct qportal_info qp_info[CONFIG_SYS_QMAN_NUM_PORTALS] = {
	/* dqrr liodn, frame data liodn, liodn off, sdest */
	SET_QP_INFO(1, 2, 1, 0),
	SET_QP_INFO(3, 4, 2, 1),
	SET_QP_INFO(5, 6, 3, 2),
	SET_QP_INFO(7, 8, 4, 3),
	SET_QP_INFO(9, 10, 5, 0),
	SET_QP_INFO(11, 12, 6, 1),
	SET_QP_INFO(13, 14, 7, 2),
	SET_QP_INFO(15, 16, 8, 3),
	SET_QP_INFO(17, 18, 9, 0),	/* for now, set sdest to 0 */
	SET_QP_INFO(19, 20, 10, 0),	/* for now, set sdest to 0 */
};
#endif

struct liodn_id_table liodn_tbl[] = {
#ifdef CONFIG_SYS_DPAA_QBMAN
	SET_QMAN_LIODN(31),
	SET_BMAN_LIODN(32),
#endif

	SET_SDHC_LIODN(1, 64),

	SET_USB_LIODN(1, "fsl-usb2-mph", 93),
	SET_USB_LIODN(2, "fsl-usb2-dr", 94),

	SET_SATA_LIODN(1, 95),
	SET_SATA_LIODN(2, 96),

	SET_PCI_LIODN_BASE(CONFIG_SYS_FSL_PCIE_COMPAT, 1, 195),
	SET_PCI_LIODN_BASE(CONFIG_SYS_FSL_PCIE_COMPAT, 2, 196),
	SET_PCI_LIODN_BASE(CONFIG_SYS_FSL_PCIE_COMPAT, 3, 197),

	SET_DMA_LIODN(1, 193),
	SET_DMA_LIODN(2, 194),
};
int liodn_tbl_sz = ARRAY_SIZE(liodn_tbl);

#ifdef CONFIG_SYS_DPAA_FMAN
struct liodn_id_table fman1_liodn_tbl[] = {
	SET_FMAN_RX_1G_LIODN(1, 0, 6),
	SET_FMAN_RX_1G_LIODN(1, 1, 7),
	SET_FMAN_RX_1G_LIODN(1, 2, 8),
	SET_FMAN_RX_1G_LIODN(1, 3, 9),
	SET_FMAN_RX_1G_LIODN(1, 4, 10),
	SET_FMAN_RX_10G_LIODN(1, 0, 11),
};
int fman1_liodn_tbl_sz = ARRAY_SIZE(fman1_liodn_tbl);

#if (CONFIG_SYS_NUM_FMAN == 2)
struct liodn_id_table fman2_liodn_tbl[] = {
	SET_FMAN_RX_1G_LIODN(2, 0, 12),
	SET_FMAN_RX_1G_LIODN(2, 1, 13),
	SET_FMAN_RX_1G_LIODN(2, 2, 14),
	SET_FMAN_RX_1G_LIODN(2, 3, 15),
	SET_FMAN_RX_1G_LIODN(2, 4, 16),
	SET_FMAN_RX_10G_LIODN(2, 0, 17),
};
int fman2_liodn_tbl_sz = ARRAY_SIZE(fman2_liodn_tbl);
#endif
#endif

struct liodn_id_table sec_liodn_tbl[] = {
	SET_SEC_JR_LIODN_ENTRY(0, 129, 130),
	SET_SEC_JR_LIODN_ENTRY(1, 131, 132),
	SET_SEC_JR_LIODN_ENTRY(2, 133, 134),
	SET_SEC_JR_LIODN_ENTRY(3, 135, 136),
	SET_SEC_RTIC_LIODN_ENTRY(a, 89),
	SET_SEC_RTIC_LIODN_ENTRY(b, 90),
	SET_SEC_RTIC_LIODN_ENTRY(c, 91),
	SET_SEC_RTIC_LIODN_ENTRY(d, 92),
	SET_SEC_DECO_LIODN_ENTRY(0, 139, 140),
	SET_SEC_DECO_LIODN_ENTRY(1, 141, 142),
	SET_SEC_DECO_LIODN_ENTRY(2, 143, 144),
	SET_SEC_DECO_LIODN_ENTRY(3, 145, 146),
};
int sec_liodn_tbl_sz = ARRAY_SIZE(sec_liodn_tbl);

#ifdef CONFIG_SYS_FSL_RAID_ENGINE
struct liodn_id_table raide_liodn_tbl[] = {
	SET_RAID_ENGINE_JQ_LIODN_ENTRY(0, 0, 60),
	SET_RAID_ENGINE_JQ_LIODN_ENTRY(0, 1, 61),
	SET_RAID_ENGINE_JQ_LIODN_ENTRY(1, 0, 62),
	SET_RAID_ENGINE_JQ_LIODN_ENTRY(1, 1, 63),
};
int raide_liodn_tbl_sz = ARRAY_SIZE(raide_liodn_tbl);
#endif

struct liodn_id_table liodn_bases[] = {
	[FSL_HW_PORTAL_SEC]  = SET_LIODN_BASE_2(64, 101),
#ifdef CONFIG_SYS_DPAA_FMAN
	[FSL_HW_PORTAL_FMAN1] = SET_LIODN_BASE_1(32),
#endif
#if (CONFIG_SYS_NUM_FMAN == 2)
	[FSL_HW_PORTAL_FMAN2] = SET_LIODN_BASE_1(160),
#endif
#ifdef CONFIG_SYS_FSL_RAID_ENGINE
	[FSL_HW_PORTAL_RAID_ENGINE]  = SET_LIODN_BASE_1(49),
#endif
};
