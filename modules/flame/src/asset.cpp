#include "asset.h"
#include "async_data.h"

#include <iostream>

namespace blaze::flame {
	Asset::Asset(std::string name, std::shared_ptr<ASyncFStream> afs, uint8_t version) : _name(name), _afs(afs), _version(version), _offset(0) {
		if (_afs && _afs->is_open()) {
			auto& fs = _afs->lock();
			fs.seekg(0, std::ios::end);
			_size = fs.tellg();
			_afs->unlock();
		} else {
			std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "File stream closed\n";
			_size = 0;
		}
	}

	Asset::Asset(std::string name, std::shared_ptr<ASyncFStream> afs, uint8_t version, uint32_t offset, uint32_t size) : _name(name), _afs(afs), _version(version), _offset(offset), _size(size) {}

	void Asset::add_load_task(std::function< TaskData(TaskData)> task) {
		_tasks.push_back(task);
	}

	// @todo We want this to turn compressed to raw data when read archive and the other way around when writing. How?
	ASyncData Asset::get_data() {
		if (!_afs || !_afs->is_open()) {
			std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "File stream closed\n";
			return ASyncData();
		} else {
			return ASyncData(_afs, _size, _offset, _tasks);
		}
	}
}
