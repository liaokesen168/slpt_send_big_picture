/*
 * calc_intf.h
 *
 *  Created on: 25 Dec, 2013
 *      Author: duke
 */

#ifndef CALC_INTF_H_
#define CALC_INTF_H_

extern std::string elf;
extern std::vector<std::string> ini_cpu;
extern std::vector<std::string> ini_ddr;
extern std::vector<struct calc_intf *> calc_intf_list;

struct calc_intf {
	calc_intf (std::string platform, std::string vendor,
			std::string cpu, std::string type, void (*func)(bool)) :
				platform(platform), vendor(vendor), cpu(cpu),
				type(type), func(func) {
		calc_intf_list.push_back(this);
	}
	std::string platform;
	std::string vendor;
	std::string cpu;
	std::string type;
	void (*func)(bool);
};


enum unit_type {
	unit_s = 0,
	unit_ms,
	unit_us,
	unit_ns,
	unit_ps,
	unit_tCK,
	unit_Hz,
	unit_KHz,
	unit_MHz,
	unit_GHz,
	unit_null
};

int ini_get_value(std::vector<std::string>&, std::string, enum unit_type);
std::string stringf(const char *fmt, ...);
void elf_write(const char *name, int value);


#include "ddr_param.h"
void ini_parse_ddr3(struct ddr3_params& ddr3_params);

#endif /* CALC_INTF_H_ */
