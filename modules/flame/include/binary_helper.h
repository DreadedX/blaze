#pragma once

#include <string>
#include <cstdint>
#include <ostream>
#include <istream>
#include <cassert>

#include "queue.h"
#include "integer.h"

namespace blaze::flame::binary {
	std::ostream& write(std::ostream& os, const std::string& value);

	std::ostream& write(std::ostream& os, const bool& value);

	std::ostream& write(std::ostream& os, const CryptoPP::Integer& value);

	std::ostream& write(std::ostream& os, const CryptoPP::ByteQueue& value);
	std::istream& read(std::istream& is, CryptoPP::ByteQueue& value, size_t length);

	std::ostream& write(std::ostream& os, const uint8_t value[], size_t length);

	std::ostream& write(std::ostream& os, const uint8_t& value);

	std::ostream& write(std::ostream& os, const uint32_t& value);
}

