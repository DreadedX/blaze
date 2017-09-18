#include "asset.h"
#include "async_data.h"

#include <iostream>

namespace blaze::flame {
	Asset::Asset(std::string name, std::shared_ptr<ASyncFStream> afs, uint16_t version) : _name(name), _afs(afs), _version(version), _offset(0), _chunk_markers(false) {
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

	Asset::Asset(std::string name, std::shared_ptr<ASyncFStream> afs, uint16_t version, uint32_t offset, uint32_t size, bool chunk_markers) : _name(name), _afs(afs), _version(version), _offset(offset), _size(size), _chunk_markers(chunk_markers) {}

	void Asset::set_workflow(Workflow workflow) {
		_workflow = workflow;
	}

	ASyncData Asset::get_data() {
		if (!_afs || !_afs->is_open()) {
			std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "File stream closed\n";
			return ASyncData();
		} else {
			return ASyncData(_afs, _size, _offset, _workflow, _chunk_markers);
		}
	}
}
