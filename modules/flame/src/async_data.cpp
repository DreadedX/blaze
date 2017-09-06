#include "async_data.h"

#include <iostream>

namespace blaze::flame {
	// @todo I don't know if makeing a copy of the task list is the best thing ever...
	Asset::TaskData async_load(std::shared_ptr<ASyncFStream> afs, uint32_t size, uint32_t offset, std::vector<std::function<Asset::TaskData(Asset::TaskData)>> tasks) {
		// @todo This works, but make_shared does not, investigate
		std::shared_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(size);

		uint32_t remaining = size;

		while (remaining > 0) {
			uint32_t chunk = remaining < CHUNK_SIZE ? remaining : CHUNK_SIZE;

			if (afs && afs->is_open()) {

				auto& fs = afs->lock();
				fs.seekg(offset + (size-remaining));
				fs.read(reinterpret_cast<char*>(data.get() + (size-remaining)), chunk);
				afs->unlock();

				remaining -= chunk;
			} else {
				// @todo This branch could cause lock ups
				std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "File stream closed\n";
				return Asset::TaskData(nullptr, 0);
			}
		}

		Asset::TaskData task_data(std::move(data), size);
		for (auto& t : tasks) {
			task_data = t(std::move(task_data));
		}

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
