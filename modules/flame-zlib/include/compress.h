#pragma once

#include "asset.h"

#include <iostream>
#include <cstring>

namespace blaze::flame::zlib {
	Asset::TaskData compress(Asset::TaskData task_data) {
		uint32_t size = task_data.second + 1;
		std::unique_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(size);
		memcpy(data.get(), task_data.first.get(), size-1);
		data[task_data.second] = 0x2e;

		Asset::TaskData new_task_data = Asset::TaskData(std::move(data), size);
		return new_task_data;
	}
}
