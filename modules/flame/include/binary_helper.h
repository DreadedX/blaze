#pragma once

#include <string>
#include <cstdint>
#include <ostream>
#include <istream>
#include <cassert>

#include "queue.h"
#include "integer.h"

namespace blaze::flame::binary {
	inline std::ostream& write(std::ostream& os, const std::string& value) {
		os << value << '\0';
		return os;
	}

	inline std::ostream& write(std::ostream& os, const bool& value) {
		os.write(reinterpret_cast<const char*>(&value), 1);
	}

	inline std::ostream& write(std::ostream& os, const CryptoPP::Integer& value) {
		size_t length = value.MinEncodedSize();
		uint8_t bytes[length];
		value.Encode(bytes, length);
		os.write(reinterpret_cast<const char*>(bytes), length);
		return os;
	}

	inline std::ostream& write(std::ostream& os, const CryptoPP::ByteQueue& value) {
		for (size_t i = 0; i < value.CurrentSize(); ++i) {
			uint8_t byte = value[i];
			os.write(reinterpret_cast<const char*>(&byte), 1);
		}
		return os;
	}
	inline std::istream& read(std::istream& is, CryptoPP::ByteQueue& value, size_t length) {
		for (size_t i = 0; i < length; ++i) {
			uint8_t byte;
			is.read(reinterpret_cast<char*>(&byte), 1);
			value.Put(byte);
		}
		return is;
	}

	inline std::ostream& write(std::ostream& os, const uint8_t value[], size_t length) {
		os.write(reinterpret_cast<const char*>(value), length);
		return os;
	}

	inline std::ostream& write(std::ostream& os, const uint8_t& value) {
		os.write(reinterpret_cast<const char*>(&value), 1);
		return os;
	}

	inline std::ostream& write(std::ostream& os, const uint32_t& value) {
		for (size_t i = 0; i < sizeof(uint32_t); ++i) {
			uint8_t byte = ((value >> i*8) & 0xff);
			os.write(reinterpret_cast<const char*>(&byte), 1);
		}
		return os;
	}
}

