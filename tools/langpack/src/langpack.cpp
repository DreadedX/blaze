#include "external_task.h"

// @todo Write my own INI-ish reader
#include "ini.hpp"

#include <iostream>

std::vector<uint8_t> langpack(std::vector<uint8_t> in) {
	std::string lang(reinterpret_cast<const char*>(in.data()));

	std::stringstream ss;
	ss << lang;

	INI::Parser p(ss);

	std::vector<uint8_t> out;
	for (auto& entry : p.top().values) {
		for (auto c : entry.first) {
			out.push_back(c);
		}
		out.push_back('\0');
		for (auto c : entry.second) {
			out.push_back(c);
		}
		out.push_back('\0');
	}

	return out;
}

EXPORT_TASK(langpack)
