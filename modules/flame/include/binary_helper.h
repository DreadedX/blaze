#pragma once

#include <string>
#include <cstdint>
#include <ostream>
#include <istream>
#include <cassert>

#include "queue.h"
#include "integer.h"

namespace blaze::flame::binary {
	bool compare(const uint8_t array1[], const uint8_t array2[], const uint32_t size);

	std::ostream& write(std::ostream& os, const std::string& value);
	std::istream& read(std::istream& is, std::string& value);

	std::ostream& write(std::ostream& os, const bool& value);

	std::ostream& write(std::ostream& os, const CryptoPP::Integer& value);

	std::ostream& write(std::ostream& os, const CryptoPP::ByteQueue& value);
	std::istream& read(std::istream& is, CryptoPP::ByteQueue& value, size_t length);

	std::ostream& write(std::ostream& os, const uint8_t value[], size_t length);
	std::istream& read(std::istream& is, uint8_t value[], size_t length);

	std::ostream& write(std::ostream& os, const uint8_t& value);
	std::istream& read(std::istream& is, uint8_t& value);

	std::ostream& write(std::ostream& os, const uint16_t& value);
	std::istream& read(std::istream& is, uint16_t& value);

	std::ostream& write(std::ostream& os, const uint32_t& value);
	std::istream& read(std::istream& is, uint32_t& value);
}

