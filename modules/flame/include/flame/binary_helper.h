#pragma once

#include "flame.h"

#include <string>
#include <cstdint>
#include <ostream>
#include <istream>
#include <cassert>
#include <array>

namespace FLAME_NAMESPACE::binary {
	// @todo Implement write/read for vectors
	bool compare(const uint8_t array1[], const uint8_t array2[], const uint32_t size);

	template <std::size_t S>
	std::ostream& write(std::ostream& os, const std::array<uint8_t, S>& value) {
		os.write(reinterpret_cast<const char*>(value.data()), value.size());
		return os;
	}
	template <std::size_t S>
	std::istream& read(std::istream& is, std::array<uint8_t, S>& value) {
		is.read(reinterpret_cast<char*>(value.data()), value.size());
		return is;
	}

	std::ostream& write(std::ostream& os, const std::string& value);
	std::istream& read(std::istream& is, std::string& value);

	std::ostream& write(std::ostream& os, const bool& value);

	std::ostream& write(std::ostream& os, const uint8_t value[], size_t length);
	std::istream& read(std::istream& is, uint8_t value[], size_t length);

	std::ostream& write(std::ostream& os, const uint8_t& value);
	std::istream& read(std::istream& is, uint8_t& value);

	std::ostream& write(std::ostream& os, const uint16_t& value);
	std::istream& read(std::istream& is, uint16_t& value);

	std::ostream& write(std::ostream& os, const uint32_t& value);
	std::istream& read(std::istream& is, uint32_t& value);
}

