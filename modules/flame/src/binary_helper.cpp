#include "binary_helper.h"

namespace blaze::flame::binary {

	bool compare(const uint8_t array1[], const uint8_t array2[], const uint32_t size) {
		for (uint32_t i = 0; i < size; ++i) {
			if (array1[i] != array2[i]) {
				return false;
			}
		}
		return true;
	}

	std::ostream& write(std::ostream& os, const std::string& value) {
		os << value << '\0';
		return os;
	}
	std::istream& read(std::istream& is, std::string& value) {
		std::getline(is, value, '\0');
		return is;
	}

	std::ostream& write(std::ostream& os, const bool& value) {
		os.write(reinterpret_cast<const char*>(&value), 1);
		return os;
	}

	std::ostream& write(std::ostream& os, const CryptoPP::Integer& value) {
		size_t length = value.MinEncodedSize();
		uint8_t bytes[length];
		value.Encode(bytes, length);
		os.write(reinterpret_cast<const char*>(bytes), length);
		return os;
	}

	std::ostream& write(std::ostream& os, const CryptoPP::ByteQueue& value) {
		for (size_t i = 0; i < value.CurrentSize(); ++i) {
			uint8_t byte = value[i];
			os.write(reinterpret_cast<const char*>(&byte), 1);
		}
		return os;
	}
	std::istream& read(std::istream& is, CryptoPP::ByteQueue& value, size_t length) {
		for (size_t i = 0; i < length; ++i) {
			uint8_t byte;
			is.read(reinterpret_cast<char*>(&byte), 1);
			value.Put(byte);
		}
		return is;
	}

	std::ostream& write(std::ostream& os, const uint8_t value[], size_t length) {
		os.write(reinterpret_cast<const char*>(value), length);
		return os;
	}
	std::istream& read(std::istream& is, uint8_t value[], size_t length) {
		is.read(reinterpret_cast<char*>(value), length);
		return is;
	}

	std::ostream& write(std::ostream& os, const uint8_t& value) {
		os.write(reinterpret_cast<const char*>(&value), 1);
		return os;
	}
	std::istream& read(std::istream& is, uint8_t& value) {
		is.read(reinterpret_cast<char*>(&value), 1);
		return is;
	}

	std::ostream& write(std::ostream& os, const uint16_t& value) {
		for (size_t i = 0; i < sizeof(uint16_t); ++i) {
			uint8_t byte = ((value >> i*8) & 0xff);
			os.write(reinterpret_cast<const char*>(&byte), 1);
		}
		return os;
	}
	std::istream& read(std::istream& is, uint16_t& value) {
		value = 0;
		for (size_t i = 0; i < sizeof(uint16_t); ++i) {
			uint8_t byte;
			is.read(reinterpret_cast<char*>(&byte), 1);
			value |= (byte << i*8);
		}
		return is;
	}

	std::ostream& write(std::ostream& os, const uint32_t& value) {
		for (size_t i = 0; i < sizeof(uint32_t); ++i) {
			uint8_t byte = ((value >> i*8) & 0xff);
			os.write(reinterpret_cast<const char*>(&byte), 1);
		}
		return os;
	}
	std::istream& read(std::istream& is, uint32_t& value) {
		value = 0;
		for (size_t i = 0; i < sizeof(uint32_t); ++i) {
			uint8_t byte;
			is.read(reinterpret_cast<char*>(&byte), 1);
			value |= (byte << i*8);
		}
		return is;
	}
}
