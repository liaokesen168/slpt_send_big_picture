/*
 *  Copyright (C) 2013 Fighter Sun <wanmyqawdr@126.com>
 *  board initialize sequence
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General  Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __JZ4780_INIT_SEQUENCE_H__
#define __JZ4780_INIT_SEQUENCE_H__

#include <initcall.h>

extern init_fnc_t init_sequence_f[];

extern init_fnc_t init_sequence_r_stage0[];
extern init_fnc_t init_sequence_r_stage1[];
extern init_fnc_t init_sequence_r_stage2[];
extern init_fnc_t init_sequence_r_stage3[];

int run_initcall_level(init_fnc_t init_sequence[], gd_t *id);

#endif
