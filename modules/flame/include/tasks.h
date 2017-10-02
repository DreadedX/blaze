#pragma once

#include "flame.h"

#include <vector>
#include <cstdint>
#include <memory>
#include <cstring>

namespace FLAME_NAMESPACE::zlib {
	std::vector<uint8_t> compress(std::vector<uint8_t> in);
	std::vector<uint8_t> decompress(std::vector<uint8_t> in);
}
