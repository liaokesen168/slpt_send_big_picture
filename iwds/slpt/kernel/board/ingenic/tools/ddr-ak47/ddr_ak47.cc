/*
 * Copyright (C) 2013 Ingenic Semiconductor Co., Ltd.
 * Authors: Duke Fong <duke@dukelec.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 3 of the License, or (at your option) any later version.
 */

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include <QtCore/QDir>
#include "calc_intf.h"

using namespace std;

string bin_path;
string elf;
vector<string> ini_cpu;
vector<string> ini_ddr;
vector<struct calc_intf *> calc_intf_list;

string stringf(const char *fmt, ...)
{
	char buf[100];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, 100, fmt, ap);
	va_end(ap);
	return buf;
}

void elf_write(const char *name, int value)
{
	string cmd = bin_path + "/../pfsv1-word-rw";
	QString path = QDir::toNativeSeparators(cmd.c_str());
	cmd = path.toStdString();
	cmd += " --file=" + elf + " --";
	cmd += stringf("%s=0x%08x", name, value);
	if (system(cmd.c_str())) {
		cerr << "ELF write error, name = " << name << endl;
		exit(-1);
	}
}

// ddr parse

void ini_parse_ddr3(struct ddr3_params& ddr3_params)
{
	ddr3_params.bl = ini_get_value(ini_ddr, "bl", unit_null);
	ddr3_params.tCL = ini_get_value(ini_ddr, "tCL", unit_tCK);
	ddr3_params.tCWL = ini_get_value(ini_ddr, "tCWL", unit_ns);
	ddr3_params.tRAS = ini_get_value(ini_ddr, "tRAS", unit_tCK);
	ddr3_params.tRP = ini_get_value(ini_ddr, "tRP", unit_ns);
	ddr3_params.tRCD = ini_get_value(ini_ddr, "tRCD", unit_ns);
	ddr3_params.tRC = ini_get_value(ini_ddr, "tRC", unit_ns);
	ddr3_params.tWR = ini_get_value(ini_ddr, "tWR", unit_ns);
	ddr3_params.tRRD = ini_get_value(ini_ddr, "tRRD", unit_ns);
	ddr3_params.tRTP = ini_get_value(ini_ddr, "tRTP", unit_tCK);
	ddr3_params.tWTR = ini_get_value(ini_ddr, "tWTR", unit_tCK);
	ddr3_params.tRFC = ini_get_value(ini_ddr, "tRFC", unit_ns);
	ddr3_params.tMINSR = ini_get_value(ini_ddr, "tMINSR", unit_null);
	ddr3_params.tXP = ini_get_value(ini_ddr, "tXP", unit_tCK);
	ddr3_params.tMRD = ini_get_value(ini_ddr, "tMRD", unit_tCK);
	ddr3_params.tCCD = ini_get_value(ini_ddr, "tCCD", unit_tCK);
	ddr3_params.tFAW = ini_get_value(ini_ddr, "tFAW", unit_ns);
	ddr3_params.tCKE = ini_get_value(ini_ddr, "tCKE", unit_tCK);
	ddr3_params.tCKSRE = ini_get_value(ini_ddr, "tCKSRE", unit_ns);
	ddr3_params.tDLLLOCK = ini_get_value(ini_ddr, "tDLLLOCK", unit_null);
	ddr3_params.tMOD = ini_get_value(ini_ddr, "tMOD", unit_null);
	ddr3_params.tXPDLL = ini_get_value(ini_ddr, "tXPDLL", unit_null);
	ddr3_params.tXS = ini_get_value(ini_ddr, "tXS", unit_ns);
	ddr3_params.tXSRD = ini_get_value(ini_ddr, "tXSRD", unit_ns);
	ddr3_params.tREFI = ini_get_value(ini_ddr, "tREFI", unit_ns);
	ddr3_params.tDLLSRST = ini_get_value(ini_ddr, "tDLLSRST", unit_null);
}

// unit conversion

static float unit_conversion(float value, enum unit_type cur_unit, enum unit_type to_unit)
{
	long long div;
	if (cur_unit != to_unit) {
		if ((to_unit == unit_s) || (to_unit == unit_ms) || (to_unit == unit_us) ||
				(to_unit == unit_ns) || (to_unit == unit_ps)) {
			if ((cur_unit != unit_s) && (cur_unit != unit_ms) &&
					(cur_unit != unit_us) && (cur_unit != unit_ns) &&
					(cur_unit != unit_ps))
				throw "need time unit!";
			div = pow (1000, abs(to_unit - cur_unit));
			if (to_unit > cur_unit)
				value *= div;
			else
				value /= div;

		}
		if ((to_unit == unit_tCK) && (cur_unit != unit_tCK))
			throw "unit must be tCK!";

		if ((to_unit == unit_Hz) || (to_unit == unit_KHz) || (to_unit == unit_MHz) ||
				(to_unit == unit_GHz)) {
			if ((cur_unit != unit_Hz) && (cur_unit != unit_KHz) &&
					(cur_unit != unit_MHz) && (cur_unit != unit_GHz))
				throw "unit must be HZ, KHz, MHz, GHz!\n";
			div = pow (1000, abs(to_unit - cur_unit));
			if (to_unit > cur_unit)
				value /= div;
			else
				value *= div;

		}
		if ((to_unit == unit_null) && (cur_unit != unit_null))
			throw "unit must be null!";
	}
	return value;
}

// ini parse

static void ini_choice_section(vector<string>& ini, string name)
{
	vector<string>::iterator it;
	bool toggle_delete = false;

	for (it = ini.begin(); it != ini.end(); /*++it*/) {

		if (it->find("[") == 0) {
			if ((it->find("ALL") == 1) || (it->find(name) == 1))
				toggle_delete = false;
			else
				toggle_delete = true;
			ini.erase(it);
			continue;
		}
		if (toggle_delete)
			ini.erase(it);
		else
			++it;
	}
}

static void ini_print_section(vector<string>& ini)
{
	string s;
	vector<string>::iterator it;
	bool virgin = true;

	for (it = ini.begin(); it != ini.end(); ++it) {
		if (it->find("[") == 0) {
			s = *it;
			s.erase(s.size() - 1).erase(0, 1);
			if (s.find("ALL") == 0)
				continue;
			if (!virgin)
				cout << ", ";
			virgin = false;
			cout << s;
		}
	}
	cout << endl;
}

static void ini_read(vector<string>& ini, string ini_file)
{
	string s;
	string::size_type pos;
	QString path = QDir::toNativeSeparators(ini_file.c_str());
	ifstream infile(path.toStdString().c_str());
	ini.clear();

	while (getline(infile, s)) {
		if ((pos = s.find("#")) != string::npos)
			s.erase(pos);
		s.erase(s.find_last_not_of(" \n\r\t") + 1);
		if (s.size() == 0)
			continue;
		ini.push_back(s);
	}
}

//

int ini_get_value(vector<string>& ini, string name, enum unit_type to_unit)
{
	// get item
	name += " =";
	vector<string>::iterator it;
	for (it = ini.begin(); it != ini.end(); ++it) {
		if (it->find(name) == 0) {
			break;
		}
	}
	if (it == ini.end()) {
		cerr << name << "> not found!\n";
		exit(-1);
	}

	// get value
	string s = it->substr(name.size());
	stringstream ss(s);
	float value;
	string unit_str;
	if (s.find("0x") != string::npos) {
		string temp;
		value = strtoul(s.c_str(), NULL, 0);
		ss >> temp;
	} else
		ss >> value;
	ss >> unit_str;
	string err_msg = "Unit error: " + name + "> ";

	enum unit_type cur_unit;

	if (unit_str.size() != 0) {
		if (unit_str.find("s") == 0)
			cur_unit = unit_s;
		else if (unit_str.find("ms") == 0)
			cur_unit = unit_ms;
		else if (unit_str.find("us") == 0)
			cur_unit = unit_us;
		else if (unit_str.find("ns") == 0)
			cur_unit = unit_ns;
		else if (unit_str.find("ps") == 0)
			cur_unit = unit_ps;
		else if (unit_str.find("tCK") == 0)
			cur_unit = unit_tCK;
		else if (unit_str.find("Hz") == 0)
			cur_unit = unit_Hz;
		else if (unit_str.find("KHz") == 0)
			cur_unit = unit_KHz;
		else if (unit_str.find("MHz") == 0)
			cur_unit = unit_MHz;
		else if (unit_str.find("GHz") == 0)
			cur_unit = unit_GHz;
		else {
			cerr << err_msg << "unknown unit!\n";
			exit(-1);
		}
	} else
		cur_unit = unit_null;

	try {
		value = unit_conversion(value, cur_unit, to_unit);
	} catch (const char* msg) {
		cerr << err_msg << msg << endl;
		exit(-1);
	}
	//cout << "value" << value << endl;
	return value + 0.5;
}

static void ini_change_value(vector<string>& ini, string name, string str)
{
	// get item
	name += " =";
	vector<string>::iterator it;
	for (it = ini.begin(); it != ini.end(); ++it) {
		if (it->find(name) == 0) {
			break;
		}
	}
	if (it == ini.end()) {
		cerr << name << "> not found!\n";
		exit(-1);
	}
	it->erase(name.size());
	*it += str;
}

static void ini_get_help(vector<string>& ini, string name, string lang)
{
	string s;
	vector<string>::iterator it;
	bool toggle_output = false;
	name += ".help." + lang + " =";

	for (it = ini.begin(); it != ini.end(); ++it) {

		if (!toggle_output) {
			if (it->find(name) == 0) {
				toggle_output = true;
				if (it->size() > name.size() + 1)
					cout << it->substr(name.size() + 1) << endl;
			}
		} else {
			if (it->find("\t") != 0)
				return;
			else
				cout << it->substr(1) << endl;
		}
	}
}

// command line interface

static void global_help(void)
{
	cout << "usage: ddr-ak47 <essential args> <normal args>\n\n";
	cout << "The essential args e.g.: (removal \"=\" to execute the query)\n";
	cout << "  --platform=xburst --vendor=ingenic --cpu=jz4780 --type=ddr3 \\\n";
	cout << "  --chip=H5TQ2G63BFR --speed=DDR-800\n\n";
	cout << "The normal args are:\n";
	cout << "  --help           This help message\n";
	cout << "  --file=FILE      Write results to ELF file\n";
	cout << "  --list           Show all cpu parameters\n";
	cout << "  --PARAMS         Explaining the cpu parameter, default english\n";
	cout << "  --lang=LANG      Explaining by other language\n";
	cout << "  --PARAMS=VALUE   Change default cpu parameters\n";
}

static void list_file(string path, bool list_dir)
{
	QDir dir(path.c_str());

	dir.setFilter((list_dir ? QDir::Dirs : QDir::Files) | QDir::NoDotAndDotDot);
	dir.setSorting(QDir::Name);

	QStringList list = dir.entryList();
	if (!list_dir)
		list.replaceInStrings(".ini", "");
	cout << list.join(", ").toStdString() << endl;
}

int main(int argc, char *argv[])
{
	string s;
	string::size_type pos;
	string platform, vendor, cpu, type;
	static void (*calc_intf_p)(bool) = NULL;

	bin_path = QFileInfo(argv[0]).absolutePath().toStdString();
	string cpu_path = bin_path + "/platform";
	string ddr_path = bin_path + "/ddr";

	// if need help
	s = argv[argc - 1];
	if (s.find("--help") == 0) {
		global_help();
		return 0;
	}

	// if write to elf
	if (s = argv[argc - 1], s.find("--file=") == 0) {
		elf = s.substr(7);
		argc--;
	}

	// check platform
	if (argc >= 2 && (s = argv[1], s.find("--platform=") == 0)) {
		platform = s.substr(11);
		cpu_path += "/" + platform;
	} else {
		cout << "Please specify --platform=PLATFORM_NAME, available platforms:\n";
		list_file(cpu_path, true);
		return 0;
	}

	// check vendor
	if (argc >= 3 && (s = argv[2], s.find("--vendor=") == 0)) {
		vendor = s.substr(9);
		cpu_path += "/" + vendor;
	} else {
		cout << "Please specify --vendor=VENDOR_NAME, available vendors:\n";
		list_file(cpu_path, true);
		return 0;
	}

	// check cpu
	if (argc >= 4 && (s = argv[3], s.find("--cpu=") == 0)) {
		cpu = s.substr(6);
		cpu_path += "/" + cpu + ".ini";
		ini_read(ini_cpu, cpu_path);
	} else {
		cout << "Please specify --cpu=CPU_NAME, available CPUs:\n";
		list_file(cpu_path, false);
		return 0;
	}

	// check type
	if (argc >= 5 && (s = argv[4], s.find("--type=") == 0)) {
		type = s.substr(7);
		ini_choice_section(ini_cpu, type);
		ddr_path += "/" + type;
	} else {
		cout << "Please specify --type=DDR_TYPE, available types:\n";
		ini_print_section(ini_cpu);
		return 0;
	}

	// check chip
	if (argc >= 6 && (s = argv[5], s.find("--chip=") == 0)) {
		ddr_path += "/" + s.substr(7) + ".ini";
		ini_read(ini_ddr, ddr_path);
	} else {
		cout << "Please specify --chip=DDR_CHIP_NAME, available chips:\n";
		list_file(ddr_path, false);
		return 0;
	}

	// check speed
	if (argc >= 7 && (s = argv[6], s.find("--speed=") == 0)) {
		ini_choice_section(ini_ddr, s.substr(8));
	} else {
		cout << "Please specify --speed=SPEED, available speed levels:\n";
		ini_print_section(ini_ddr);
		return 0;
	}

	// choice calculate interface

	vector<struct calc_intf *>::iterator it;
	for (it = calc_intf_list.begin(); it != calc_intf_list.end(); ++it) {
		if (((*it)->platform == platform) && ((*it)->vendor == vendor) &&
				((*it)->cpu == cpu) && ((*it)->type == type)) {
			calc_intf_p = (*it)->func;
			break;
		}
	}
	if (it == calc_intf_list.end()) {
		cerr << "Error: calculate function is not yet implemented!\n";
		exit(-1);
	}

	// if specify --list
	if (argc >= 8 && (s = argv[argc - 1], s.find("--list") == 0)) {
		(*calc_intf_p)(false);
		return 0;
	}

	// if get parameters info
	if (argc >= 8 && (s = argv[7], s.find("=") == string::npos)) {
		string name = s.substr(2);
		string lang;
		if (argc >= 9 && (s = argv[8], s.find("--lang=") == 0))
			lang = s.substr(7);
		else
			lang = "en";
		ini_get_help(ini_cpu, name, lang);
		return 0;
	}

	for (int i = 7; i < argc; ++i) {
		s = argv[i];
		pos = s.find("=");
		if (pos == string::npos) {
			cerr << "Error: can't change value without \"=\"\n";
			exit(-1);
		}
		string name = s.substr(2, pos - 2);
		string str = s.substr(pos + 1);
		ini_change_value(ini_cpu, name, str);
	}

	cout << "Calculated as follows:\n";
	(*calc_intf_p)(true);

	return 0;
}
