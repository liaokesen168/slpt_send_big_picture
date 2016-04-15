/*
 * Copyright (C) 2014 Ingenic Semiconductor Co., Ltd.
 * Authors: Kage Shen <kkshen@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the License, or (at your
 * option) any later version.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _SLPT_APP_H_
#define _SLPT_APP_H_

#include <initcall.h>
#include <slpt.h>

/* init/exit onetime */
#define SLPT_ARCH_INIT_ONETIME(func)                                   \
	__attribute__ ((__used__, __section__(".slpt_archcall_onetime")))  \
	static init_fnc_t init_call_onetime_##func = (init_fnc_t)func

#define SLPT_CORE_INIT_ONETIME(func)                                   \
	__attribute__ ((__used__, __section__(".slpt_corecall_onetime")))  \
	static init_fnc_t init_call_onetime_##func = (init_fnc_t)func

#define SLPT_APP_INIT_ONETIME(func)                                    \
	__attribute__ ((__used__, __section__(".slpt_initcall_onetime")))  \
	static init_fnc_t init_call_onetime_##func = (init_fnc_t)func

#define SLPT_ARCH_EXIT_ONETIME(func)                                   \
	__attribute__ ((__used__, __section__(".slpt_arch_exit_onetime"))) \
	static init_fnc_t exit_call_onetime_##func = (init_fnc_t)func

#define SLPT_CORE_EXIT_ONETIME(func)                                   \
	__attribute__ ((__used__, __section__(".slpt_core_exit_onetime"))) \
	static init_fnc_t exit_call_onetime_##func = (init_fnc_t)func

#define SLPT_APP_EXIT_ONETIME(func)                                    \
	__attribute__ ((__used__, __section__(".slpt_init_exit_onetime"))) \
	static init_fnc_t exit_call_onetime_##func = (init_fnc_t)func

/* init/exit everytime */
#define SLPT_ARCH_INIT_EVERYTIME(func)                                 \
	__attribute__ ((__used__, __section__(".slpt_archcall_everytime")))\
	static init_fnc_t init_call_everytime_##func = (init_fnc_t)func

#define SLPT_CORE_INIT_EVERYTIME(func)                                 \
	__attribute__ ((__used__, __section__(".slpt_corecall_everytime")))\
	static init_fnc_t init_call_everytime_##func = (init_fnc_t)func

#define SLPT_APP_INIT_EVERYTIME(func)                                  \
	__attribute__ ((__used__, __section__(".slpt_initcall_everytime")))\
	static init_fnc_t init_call_everytime_##func = (init_fnc_t)func

#define SLPT_ARCH_EXIT_EVERYTIME(func)                                  \
	__attribute__ ((__used__, __section__(".slpt_arch_exit_everytime")))\
	static init_fnc_t exit_call_everytime_##func = (init_fnc_t)func

#define SLPT_CORE_EXIT_EVERYTIME(func)                                  \
	__attribute__ ((__used__, __section__(".slpt_core_exit_everytime")))\
	static init_fnc_t exit_call_everytime_##func = (init_fnc_t)func

#define SLPT_APP_EXIT_EVERYTIME(func)                                  \
	__attribute__ ((__used__, __section__(".slpt_exitcall_everytime")))\
	static init_fnc_t exit_call_everytime_##func = (init_fnc_t)func

extern void do_gokernel(void);
extern void do_suspend(int);

extern int slpt_mode_exit(void);
extern int slpt_mode_shutdown(void);

void slpt_initcall_onetime(void);
void slpt_exitcall_onetime(void);

#endif
