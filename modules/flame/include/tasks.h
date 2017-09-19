#pragma once

#include <utility>
#include <cstdint>
#include <memory>
#include <cstring>

namespace blaze::flame {
	std::pair<std::unique_ptr<uint8_t[]>, uint32_t> add_chunk_marker(std::pair<std::unique_ptr<uint8_t[]>, uint32_t> info) {
		auto data = std::make_unique<uint8_t[]>(info.second + sizeof(info.second));
		for (size_t i = 0; i < sizeof(info.second); ++i) {
			data[i] = ((info.second >> i*8) & 0xff);
		}
		memcpy(data.get() + sizeof(info.second), info.first.get(), info.second);

		return std::make_pair(std::move(data), info.second + sizeof(info.second));
	}
}
