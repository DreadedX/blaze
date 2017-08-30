#include "asset.h"

#include "async_fstream.h"

// @todo Is this a good number?
#define CHUNK_SIZE 1024

std::vector<std::thread> blaze::flame::Asset::_workers;

// @todo We want this to turn compressed to raw data when read archive and the other way around when writing. How?
std::shared_ptr<blaze::flame::ASyncData> blaze::flame::Asset::get_data() {
	auto async_data = _async_data.lock();
	if (!async_data) {
		async_data = std::make_shared<ASyncData>();
		_async_data = async_data;
	}

	// @todo Make this load from the actual file, check if it is not an nullptr
	if (async_data->get_state() == State::NOT_LOADED && _afs && _afs->is_open()) {
		async_data->set_state(State::LOADING);
		// @note Lock a mutex to prevent the Asset from going out of scope before the thread is done
		_mutex.lock();
		_workers.push_back(std::thread([this](){
			uint32_t size = _size;
			// @todo This works, but make_shared does not, investigate
			std::shared_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(size);

			uint32_t remaining = _size;

			while (remaining > 0) {
				uint32_t chunk = remaining < CHUNK_SIZE ? remaining : CHUNK_SIZE;

				if (_afs && _afs->is_open()) {

					auto& fs = _afs->lock();
					fs.seekg(_offset + (size-remaining));
					fs.read(reinterpret_cast<char*>(data.get() + (size-remaining)), chunk);
					_afs->unlock();

					remaining -= chunk;
				} else {
					// @todo This branch could cause lock ups
					std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "File stream closed\n";
					_mutex.unlock();
					return;
				}
			}

			TaskData task_data(std::move(data), size);
			for (auto& t : _tasks) {
				task_data = t(std::move(task_data));
			}

			auto async_data = _async_data.lock();
			// Make sure the async data object has not gone out of use
			if (async_data) {
				async_data->set_data(std::move(task_data.first), task_data.second);
			} else {
				std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "Data closed\n";
			}
			_mutex.unlock();
		}));
	}

	if (!_afs || !_afs->is_open()) {
		std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "File stream closed\n";
	}

	return async_data;
}
