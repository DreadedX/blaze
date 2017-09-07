#include "async_data.h"

#include <iostream>
#include <cstring>

namespace blaze::flame {
	Asset::TaskData async_load(std::shared_ptr<ASyncFStream> afs, uint32_t size, uint32_t offset, std::vector<std::function<Asset::TaskData(Asset::TaskData)>> tasks) {
		// The final data array grows each iteration of the loop with the size of the processed data, this is because we do not know what the final size will be
		std::unique_ptr<uint8_t[]> data = nullptr;

		uint32_t data_pos = 0;
		uint32_t remaining = size;

		// @todo This inner loop can be reused for streaming data, e.g. music
		while (remaining > 0) {
			uint32_t chunk = remaining < CHUNK_SIZE ? remaining : CHUNK_SIZE;

			if (afs && afs->is_open()) {

				std::unique_ptr<uint8_t[]> tmp_data = std::make_unique<uint8_t[]>(chunk);

				auto& fs = afs->lock();
				fs.seekg(offset + (size-remaining));
				fs.read(reinterpret_cast<char*>(tmp_data.get()), chunk);
				afs->unlock();
	
				// @todo Pass task type enum
				// Asset::TaskData task_data(std::move(tmp_data), chunk, TaskDataType::STREAM);
				// @todo Small problem, we do not know the size of e.g. the compressed data chunks, how are we going to deal with that
				Asset::TaskData task_data(std::move(tmp_data), chunk);
				for (auto& t : tasks) {
					task_data = t(std::move(task_data));
				}
				
				std::unique_ptr<uint8_t[]> current_data = std::move(data);
				data = std::make_unique<uint8_t[]>(data_pos + task_data.second);
				if (current_data != nullptr) {
					memcpy(data.get(), current_data.get(), data_pos);
				}
				memcpy(data.get() + data_pos, task_data.first.get(), task_data.second);

				data_pos += task_data.second;
				remaining -= chunk;
			} else {
				std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "File stream closed\n";
				return Asset::TaskData(nullptr, 0);
			}
		}

		// @todo Pass in an task type enum, then we can enable this again
		// Asset::TaskData task_data(std::move(data), data_pos, TaskDataType::FINAL);
		Asset::TaskData task_data(std::move(data), data_pos);
		// for (auto& t : tasks) {
		// 	task_data = t(std::move(task_data));
		// }

		return task_data;
	}

	ASyncData::ASyncData(std::shared_ptr<ASyncFStream> afs, uint32_t size, uint32_t offset, std::vector<std::function<Asset::TaskData(Asset::TaskData)>> tasks) {
		_future = std::async(std::launch::async, async_load, afs, size, offset, tasks);
	}
	ASyncData::ASyncData() {
		_state = State::FAILED;
		_task_data = Asset::TaskData(nullptr, 0);
	}

	State ASyncData::get_state() {
		// @todo Is _future invalid after calling get, if so we do not have to check the state
		if (_state == State::LOADING && _future.valid() && _future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
			_task_data = _future.get();
			if (_task_data.first == nullptr) {
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
		return _task_data.second;
	}

	// @note Never store the result of this function, as ASyncData going out of scope deletes it
	uint8_t* ASyncData::data() {
		_wait_until_loaded();
		return _task_data.first.get();
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
