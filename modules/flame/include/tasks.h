#pragma once

#include "flame.h"

#include <utility>
#include <cstdint>
#include <memory>
#include <cstring>

namespace FLAME_NAMESPACE::zlib {
	std::pair<std::unique_ptr<uint8_t[]>, uint32_t> compress(std::pair<std::unique_ptr<uint8_t[]>, uint32_t> info);
	std::pair<std::unique_ptr<uint8_t[]>, uint32_t> decompress(std::pair<std::unique_ptr<uint8_t[]>, uint32_t> info);
}
