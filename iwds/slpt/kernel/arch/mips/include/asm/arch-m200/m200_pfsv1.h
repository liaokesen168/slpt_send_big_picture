/*
 *  Copyright (C) 2013 Fighter Sun <wanmyqawdr@126.com>
 *  helpers for implement of parameterized firmware specification v1
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

#ifndef M200_PFSV1_H
#define M200_PFSV1_H

#define PFSV1_BRIEF_VALUE_RANGE_GPIO            \
    "[0   :  31](\"GPIO_PA\"%1),"               \
    "[32  :  63](\"GPIO_PB\"(%1-32)),"          \
    "[64  :  95](\"GPIO_PC\"(%1-64)),"          \
    "[96  :  127](\"GPIO_PD\"(%1-96)),"         \
    "[128 :  159](\"GPIO_PE\"(%1-128)),"        \
    "[160 :  191](\"GPIO_PF\"(%1-160))"

#endif
