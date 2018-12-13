#include "flame/asset_data.h"

#include <iostream>
#include <fstream>
#include <cstring>

#define CHUNK_SIZE 16384

namespace FLAME_NAMESPACE {
	DataLoader::DataLoader(std::future<std::vector<uint8_t>> future, bool async) : _future(std::move(future)), _async(async) {}

	bool DataLoader::is_valid() {
		return _future.valid();
	}

	bool DataLoader::is_loaded() {
		return is_valid() && (_async || _future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready);
	}

	void DataLoader::wait() {
		_future.wait();
	}

	// This function can only be called once after that the it will become invalid
	std::vector<uint8_t> DataLoader::get() {
		if (is_loaded()) {
			return _future.get();
		} else if (is_valid()) {
			throw std::runtime_error("DataLoader is not loaded!");
		} else {
			throw std::runtime_error("DataLoader is not valid!");
		}
	}

	template<>
	std::string DataLoader::get_as<std::string>() {
		auto data = get();
		return std::string(data.begin(), data.end());
	}
}
