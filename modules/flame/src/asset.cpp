#include "asset.h"

#include <iostream>

// @todo Is this a good number?
#define CHUNK_SIZE 1024

namespace blaze::flame {
	Asset::Asset(std::string name, std::shared_ptr<ASyncFStream> afs, uint8_t version, uint32_t offset) : _name(name), _afs(afs), _version(version), _offset(offset) {
		if (_afs && _afs->is_open()) {
			auto& fs = _afs->lock();
			auto pos = fs.tellg();
			fs.seekg(0, std::ios::end);
			_size = fs.tellg();
			fs.seekg(pos);
			_afs->unlock();
		} else {
			std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "File stream closed\n";
			_size = 0;
		}
	}

	void Asset::add_load_task(std::function< TaskData(TaskData)> task) {
		_tasks.push_back(task);
	}

	// @todo I don't know if makeing a copy of the task list is the best thing ever...
	void async_load(std::shared_ptr<ASyncFStream> afs, std::shared_ptr<ASyncData> async_data, uint32_t size, uint32_t offset, std::vector<std::function<Asset::TaskData(Asset::TaskData)>> tasks) {
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
				async_data->set_state(State::FAILED);
				return;
			}
		}

		Asset::TaskData task_data(std::move(data), size);
		for (auto& t : tasks) {
			task_data = t(std::move(task_data));
		}

		async_data->set_data(std::move(task_data.first), task_data.second);

		return;
	}

	// @todo We want this to turn compressed to raw data when read archive and the other way around when writing. How?
	std::shared_ptr<ASyncData> Asset::get_data() {
		auto async_data = _async_data.lock();
		if (!async_data) {
			async_data = std::make_shared<ASyncData>();
			_async_data = async_data;
		}

		if (!_afs || !_afs->is_open()) {
			std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "File stream closed\n";
			async_data->set_state(State::FAILED);
		}

		// @todo Make this load from the actual file, check if it is not an nullptr
		if (async_data->get_state() == State::NOT_LOADED && _afs && _afs->is_open()) {
			async_data->set_state(State::LOADING);
			_future = std::async(std::launch::async, async_load, _afs, async_data, _size, _offset, _tasks);
		}

		return async_data;
	}
}
