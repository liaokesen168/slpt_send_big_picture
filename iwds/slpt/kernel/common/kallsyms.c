/*
 * Helper functions for working with the builtin symbol table
 *
 * Copyright (c) 2008-2009 Analog Devices Inc.
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <linux/ctype.h>

/* We need the weak marking as this symbol is provided specially */
extern const char system_map[] __attribute__((weak));

static unsigned long sym_strtoul(const char *cp, char **endp, unsigned int base)
{
	u64 result = 0, value;

	while (!(result & 1ULL << 63) && isxdigit(*cp)
			&& (value = isdigit(*cp) ? *cp-'0' : (islower(*cp)
			? toupper(*cp) : *cp)-'A'+10) < base) {
		result = result*base + value;
		cp++;
	}
	if (endp)
		*endp = (char *)cp;
	return result & 0xffffffff;
}

/* Given an address, return a pointer to the symbol name and store
 * the base address in caddr.  So if the symbol map had an entry:
 *		03fb9b7c_spi_cs_deactivate
 * Then the following call:
 *		unsigned long base;
 *		const char *sym = symbol_lookup(0x03fb9b80, &base);
 * Would end up setting the variables like so:
 *		base = 0x03fb9b7c;
 *		sym = "_spi_cs_deactivate";
 */
const char *symbol_lookup(unsigned long addr, unsigned long *caddr)
{
	const char *sym, *csym;
	char *esym;
	unsigned long sym_addr;

	sym = system_map;
	csym = NULL;
	*caddr = 0;

	while (*sym) {
		sym_addr = sym_strtoul(sym, &esym, 16);
		sym = esym;
		if (sym_addr > addr)
			break;
		*caddr = sym_addr;
		csym = sym;
		sym += strlen(sym) + 1;
	}

	return csym;
}

const char *kallsyms_lookup(unsigned long addr,
			    unsigned long *caddr,
			    unsigned long *size)
{
	const char *sym, *csym;
	char *esym;
	unsigned long sym_addr;

	if (addr >= text_copy_end())
		return NULL;

	sym = system_map;
	csym = NULL;
	*caddr = sym_addr = 0;

	while (*sym) {
		sym_addr = sym_strtoul(sym, &esym, 16);
		sym = esym;
		if (sym_addr > addr)
			break;
		*caddr = sym_addr;
		csym = sym;
		sym += strlen(sym) + 1;
	}

	if (sym_addr >= text_copy_end())
		sym_addr = text_copy_end();

	*size = sym_addr - *caddr;
	return csym;
}

/* Look up a kernel symbol and return it in a text buffer. */
int sprint_symbol(char *buffer, unsigned long address)
{
	const char *name;
	unsigned long caddr, size;
	int len;

	name = kallsyms_lookup(address, &caddr, &size);
	if (!name)
		return sprintf(buffer, "0x%lx", address);

	strcpy(buffer, name);
	len = strlen(buffer);
	len += sprintf(buffer + len, "+%#lx/%#lx", address - caddr, size);

	return len;
}
