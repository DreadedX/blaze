#include "asset_data.h"

#include "binary_helper.h"

#include <iostream>
#include <cstring>

#define CHUNK_SIZE 16384

namespace FLAME_NAMESPACE {
	std::pair<std::unique_ptr<uint8_t[]>, uint32_t> async_load(std::shared_ptr<FileHandler> fh, uint32_t size, uint32_t offset, MetaAsset::Workflow workflow) {
		std::unique_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(size);

		uint32_t remaining = size;

		while (remaining > 0) {
			if (fh && fh->is_open()) {
				auto& fs = fh->lock();
				fs.seekg(offset + (size-remaining));

				uint32_t chunk = remaining < CHUNK_SIZE ? remaining : CHUNK_SIZE;

				fs.read(reinterpret_cast<char*>(data.get() + (size-remaining) ), chunk);
				fh->unlock();
	
				remaining -= chunk;
			} else {
				// Instead of an exception we return a nullptr and throw the exception from is_loaded
				return std::make_pair(nullptr, 0);
			}
		}

		auto info = std::make_pair(std::move(data), size);
		for (auto& t : workflow.tasks) {
			info = t(std::move(info));
		}

		return info;
	}

	AssetData::AssetData(std::shared_ptr<FileHandler> fh, uint32_t size, uint32_t offset, MetaAsset::Workflow workflow) {
		_future = std::async(std::launch::async, async_load, fh, size, offset, workflow);
	}

	bool AssetData::is_loaded() {
		// @todo Zero size file give an error, do not know if that is the correct thing
		if (!_loaded && _future.valid() && _future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
			auto info = _future.get();
			_data = std::move(info.first);
			_size = info.second;
			if (_data == nullptr) {
				throw std::runtime_error("Failed to load data");
			} else {
				_loaded = true;
			}
		}
		return _loaded;
	}
	uint32_t AssetData::get_size() {
		_wait_until_loaded();
		return _size;
	}

	// @note Never store the result of this function, as AssetData going out of scope deletes it
	uint8_t* AssetData::data() {
		_wait_until_loaded();
		return _data.get();
	}

	uint8_t& AssetData::operator[](uint32_t idx) {
		// @todo Do we need to check this even in release mode
		if (idx >= get_size()) {
			throw std::out_of_range("Array out of bounds");
		}
		return data()[idx];
	}

	void AssetData::_wait_until_loaded() {
		if (!_loaded && _future.valid()) {
			_future.wait();
			is_loaded();
		}
	}
}
