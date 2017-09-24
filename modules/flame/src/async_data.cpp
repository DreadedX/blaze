#include "async_data.h"

#include "binary_helper.h"

#include <iostream>
#include <cstring>

#define CHUNK_SIZE 16384

namespace blaze::flame {
	// @todo Make chunk marker optional in asset/archive
	// @todo Outside of compression in chunks, is there really any good reason for the chunk markers, depends on how audio gets stored and if the chunk size for this is predictable
	std::pair<std::unique_ptr<uint8_t[]>, uint32_t> async_load(std::shared_ptr<ASyncFStream> afs, uint32_t size, uint32_t offset, Asset::Workflow workflow) {
		// The final data array grows each iteration of the loop with the size of the processed data, this is because we do not know what the final size will be
		std::unique_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(size);

		uint32_t remaining = size;

		// @todo This inner loop can be reused for streaming data, e.g. music
		// @todo We are doing a lot of allocating here
		while (remaining > 0) {
			if (afs && afs->is_open()) {
				auto& fs = afs->lock();
				fs.seekg(offset + (size-remaining));

				uint32_t chunk = remaining < CHUNK_SIZE ? remaining : CHUNK_SIZE;
				std::unique_ptr<uint8_t[]> tmp_data = std::make_unique<uint8_t[]>(chunk);

				fs.read(reinterpret_cast<char*>(data.get() + (size-remaining) ), chunk);
				afs->unlock();
	
				remaining -= chunk;
			} else {
				std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "File stream closed\n";
				return std::make_pair(nullptr, 0);
			}
		}

		auto info = std::make_pair(std::move(data), size);
		for (auto& t : workflow.tasks) {
			info = t(std::move(info));
		}

		return info;
	}

	ASyncData::ASyncData(std::shared_ptr<ASyncFStream> afs, uint32_t size, uint32_t offset, Asset::Workflow workflow) {
		_future = std::async(std::launch::async, async_load, afs, size, offset, workflow);
	}
	ASyncData::ASyncData() {
		_state = State::FAILED;
		_data = nullptr;
		_size = 0;
	}

	State ASyncData::get_state() {
		// @todo Is _future invalid after calling get, if so we do not have to check the state
		// @todo Zero size file give an error, do not know if that is the correct thing
		if (_state == State::LOADING && _future.valid() && _future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
			auto info = _future.get();
			_data = std::move(info.first);
			_size = info.second;
			if (_data == nullptr) {
				_state = State::FAILED;
				std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "Failed to load\n";
			} else {
				_state = State::LOADED;
			}
		}
		return _state;
	}
	bool ASyncData::is_loaded() {
		return get_state() == State::LOADED;
	}
	uint32_t ASyncData::get_size() {
		_wait_until_loaded();
		return _size;
	}

	// @note Never store the result of this function, as ASyncData going out of scope deletes it
	uint8_t* ASyncData::data() {
		_wait_until_loaded();
		return _data.get();
	}

	uint8_t& ASyncData::operator[](uint32_t idx) {
		#if DEBUG
			if (idx >= get_size()) {
				std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "Array out of bounds\n";
			}
		#endif
		return data()[idx];
	}

	void ASyncData::_wait_until_loaded() {
		if (_state == State::LOADING && _future.valid()) {
			_future.wait();
			get_state();
		}
	}
}
