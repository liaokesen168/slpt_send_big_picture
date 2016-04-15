/*
 * Copyright (c) 2013 Ingenic Semiconductor, <lhhuang@ingenic.cn>
 * 
 * @references: borrowed heavily from Android bionic code:
 *  bionic/libc/string/strerror.c
 *  bionic/libc/string/strerror_r.c
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

#include <asm/errno.h>
#include <linux/string.h>

typedef struct {
  int           code;
  const char*   msg;
} CodeString;

static const char*
__code_string_lookup( const CodeString*  strings,
                      int                code )
{
    int  nn = 0;

    for (;;)
    {
        if (strings[nn].msg == NULL)
            break;

        if (strings[nn].code == code)
            return strings[nn].msg;

        nn++;
    }
    return NULL;
}


static const CodeString  _sys_error_strings[] =
{
#define  __ERRDEF(x,y,z)  { x, z },
#include "_errdefs.h"
    { 0, NULL }
};

static size_t
__digits10(unsigned int num)
{
	size_t i = 0;

	do {
		num /= 10;
		i++;
	} while (num != 0);

	return i;
}

static int
__itoa(int num, int sign, char *buffer, size_t start, size_t end)
{
	size_t pos;
	unsigned int a;
	int neg;

	if (sign && num < 0) {
		a = -num;
		neg = 1;
	}
	else {
		a = num;
		neg = 0;
	}

	pos = start + __digits10(a);
	if (neg)
	    pos++;

	if (pos < end)
		buffer[pos] = '\0';
	else
		return ERANGE;
	pos--;
	do {
		buffer[pos] = (a % 10) + '0';
		pos--;
		a /= 10;
	} while (a != 0);
	if (neg)
		buffer[pos] = '-';
	return 0;
}


static int
strerror_r(int errnum, char *strerrbuf, size_t buflen)
{
    int          ret = 0;
    const char*  msg;

    msg        = __code_string_lookup( _sys_error_strings, errnum );
    if (msg != NULL) {
        strncpy(strerrbuf, msg, buflen);
        if ((size_t)strlen(msg) >= buflen)
            ret = ERANGE;
    } else {
        strncpy(strerrbuf, "Unknown error: ", buflen);

        int ret = __itoa(errnum, 1, strerrbuf, 15, buflen);

        if (ret == 0)
        	ret = EINVAL;
    }
    return ret;
}

char *
strerror(int num)
{
	static char buf[256];

	(void)strerror_r(-num, buf, sizeof(buf));
	return (buf);
}
