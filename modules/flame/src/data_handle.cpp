#include "flame/data_handle.h"

#include <iostream>
#include <fstream>
#include <cstring>

#define CHUNK_SIZE 16384

namespace FLAME_NAMESPACE {
	DataHandle::DataHandle(std::future<std::vector<uint8_t>> future, bool async) : _future(std::move(future)), _async(async) {}

	bool DataHandle::is_valid() {
		return _future.valid();
	}

	bool DataHandle::is_loaded() {
		return is_valid() && (!_async || _future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready);
	}

	void DataHandle::wait() {
		_future.wait();
	}

	// This function can only be called once after that the it will become invalid
	std::vector<uint8_t> DataHandle::get() {
		if (is_loaded()) {
			return _future.get();
		} else if (is_valid()) {
			throw std::runtime_error("DataLoader is not loaded!");
		} else {
			throw std::runtime_error("DataLoader is not valid!");
		}
	}

	template<>
	std::string DataHandle::get_as<std::string>() {
		auto data = get();
		return std::string(data.begin(), data.end());
	}
}
