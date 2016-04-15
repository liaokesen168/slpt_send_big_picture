/*
 *  Copyright (C) 2013 Fighter Sun <wanmyqawdr@126.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <common.h>
#include <pfsv1.h>

extern struct pfsv1_param_desc __pfsv1_param_desc_start[];
extern struct pfsv1_param_desc __pfsv1_param_desc_end[];

extern struct pfsv1_help_utf8_desc __pfsv1_help_zh_cn_utf8_start[];
extern struct pfsv1_help_utf8_desc __pfsv1_help_zh_cn_utf8_end[];

extern struct pfsv1_help_utf8_desc __pfsv1_help_en_us_utf8_start[];
extern struct pfsv1_help_utf8_desc __pfsv1_help_en_us_utf8_end[];

static int do_pfsv1info(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i;

	printf("=================================\n");
	printf("Dump PFSV1 parameters descriptor:\n");
	printf("=================================\n");
	for (i = 0; &__pfsv1_param_desc_start[i] < __pfsv1_param_desc_end; i++)
		printf("[%d]: \"%s\", \"%s\", \"%s\", 0x%x, %u, \"%s\", \"%s\"\n",
				i,
				__pfsv1_param_desc_start[i].name,
				__pfsv1_param_desc_start[i].alias,
				__pfsv1_param_desc_start[i].type,
				(unsigned int)__pfsv1_param_desc_start[i].addr,
				(unsigned int)__pfsv1_param_desc_start[i].size,
				__pfsv1_param_desc_start[i].perm,
				__pfsv1_param_desc_start[i].brief);

	printf("\n\n=================================\n");
	printf("Dump PFSV1 help zh_CN.utf8:\n");
	printf("=================================\n");
	for (i = 0; &__pfsv1_help_zh_cn_utf8_start[i] < __pfsv1_help_zh_cn_utf8_end; i++)
		printf("[%d]: \"%s\", \"%s\"\n",
				i,
				__pfsv1_help_zh_cn_utf8_start[i].name,
				__pfsv1_help_zh_cn_utf8_start[i].text);

	printf("\n\n=================================\n");
	printf("Dump PFSV1 help en_US.utf8:\n");
	printf("=================================\n");
	for (i = 0; &__pfsv1_help_en_us_utf8_start[i] < __pfsv1_help_en_us_utf8_end; i++)
		printf("[%d]: \"%s\", \"%s\"\n",
				i,
				__pfsv1_help_en_us_utf8_start[i].name,
				__pfsv1_help_en_us_utf8_start[i].text);

	return 0;
}

U_BOOT_CMD(
    pfsv1info, 1, 0, do_pfsv1info,
    "show PFSV1 data",
    "cmd: platforminfo"
);
