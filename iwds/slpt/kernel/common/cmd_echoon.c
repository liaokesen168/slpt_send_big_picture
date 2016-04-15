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

extern int _is_echo_on;

static int do_echoon(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    if (argc < 2)
        return -1;

    if (!strcmp(argv[1], "0"))
        _is_echo_on = 0;
    else if (!strcmp(argv[1], "1"))
        _is_echo_on = 1;
    else
        return -1;

    return 0;
}

U_BOOT_CMD(
    echoon, 2, 0, do_echoon,
    "dis/enable console echo",
    "\nechoon 0 --- disable echo\n"
    "echoon 1 --- enable echo"
);
