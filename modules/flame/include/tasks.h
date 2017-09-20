#pragma once

#include <utility>
#include <cstdint>
#include <memory>
#include <cstring>

namespace blaze::flame {
	std::pair<std::unique_ptr<uint8_t[]>, uint32_t> add_chunk_marker(std::pair<std::unique_ptr<uint8_t[]>, uint32_t> info);
}
