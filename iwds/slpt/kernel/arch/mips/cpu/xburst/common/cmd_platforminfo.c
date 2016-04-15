/*
 *  Copyright (C) 2013 Fighter Sun <wanmyqawdr@126.com>
 *  functions for board special parameters
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
#include <command.h>

#include <linux/compat.h>

#ifndef CONFIG_SOC_NAME
#error "Must define CONFIG_SOC_NAME=\"$(SOC)\""
#endif

static int xburst_show_platform_info(cmd_tbl_t *cmdtp, int flag, int argc,
        char * const argv[])
{
    printk("name: "CONFIG_SOC_NAME"\n");
    return 0;
}

U_BOOT_CMD(
    platforminfo, 1, 0, xburst_show_platform_info,
    "show platform [cpuname]",
    "cmd: platforminfo"
);
