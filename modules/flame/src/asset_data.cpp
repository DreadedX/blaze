#include "flame/asset_data.h"

#include <iostream>
#include <fstream>
#include <cstring>

#define CHUNK_SIZE 16384

namespace FLAME_NAMESPACE {
	std::vector<uint8_t> async_load(std::string filename, size_t size, size_t offset, std::vector<MetaAsset::Task> workflow, std::function<void(std::vector<uint8_t>)> callback) {
		std::vector<uint8_t> data(size);

		std::fstream fs(filename, std::ios::in | std::ios::binary);
		if (!fs.is_open()) {
			throw std::runtime_error("ASYNC: Failed to open file");
		}

		fs.seekg(offset);
		fs.read(reinterpret_cast<char*>(data.data()), size);

		for (auto& t : workflow) {
			data = t(std::move(data));
		}

		if (callback != nullptr) {
			callback(data);
		}

		return data;
	}

	AssetData::AssetData(std::string filename, size_t size, size_t offset, std::vector<MetaAsset::Task> workflow, bool async, std::function<void(std::vector<uint8_t>)> callback) : _async(async) {
		std::launch policy = _async ? std::launch::async : std::launch::deferred;
		_future = std::async(policy, async_load, filename, size, offset, workflow, callback);
	}

	bool AssetData::is_loaded() {
		if (!_loaded && _future.valid() && ( (_future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) || !_async )) {
			_data = _future.get();
			_loaded = true;
		}
		return _loaded;
	}

	size_t AssetData::get_size() {
		_wait_until_loaded();
		return _data.size();
	}

	template<>
	std::string AssetData::as() {
		_wait_until_loaded();
		return std::string(_data.begin(), _data.end());
	}

	// @note This is kind of a dangerous function as we return the actual data
	template<>
	uint8_t* AssetData::as() {
		_wait_until_loaded();
		return _data.data();
	}

	template<>
	std::vector<uint8_t>& AssetData::as() {
		_wait_until_loaded();
		return _data;
	}


	uint8_t& AssetData::operator[](uint32_t idx) {
		if (idx >= get_size()) {
			throw std::out_of_range("Array out of bounds");
		}

		_wait_until_loaded();
		return _data.at(idx);
	}

	void AssetData::_wait_until_loaded() {
		if (!_loaded && _future.valid()) {
			_future.wait();
			is_loaded();
		}
	}
}
