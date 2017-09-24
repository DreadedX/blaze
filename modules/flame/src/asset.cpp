#include "asset.h"
#include "async_data.h"

#include <iostream>

namespace blaze::flame {
	Asset::Asset(std::string name, std::shared_ptr<ASyncFStream> afs, uint16_t version, Workflow workflow) : _name(name), _afs(afs), _version(version), _offset(0), _chunk_markers(false), _base_workflow(workflow) {
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

	Asset::Asset(std::string name, std::shared_ptr<ASyncFStream> afs, uint16_t version, uint32_t offset, uint32_t size, bool chunk_markers, Workflow workflow) : _name(name), _afs(afs), _version(version), _offset(offset), _size(size), _chunk_markers(chunk_markers), _base_workflow(workflow) {}

	const std::string& Asset::get_name() const {
		return _name;
	}

	uint16_t Asset::get_version() const {
		return _version;
	}

	ASyncData Asset::get_data(Workflow workflow) {

		// Construct a new workflow based on the base workflow and given workflow
		// @todo Improve this
		Workflow final_workflow;
		for (auto t :_base_workflow.inner) {
			final_workflow.inner.push_back(t);
		}
		for (auto t :_base_workflow.outer) {
			final_workflow.outer.push_back(t);
		}
		for (auto t : workflow.inner) {
			final_workflow.inner.push_back(t);
		}
		for (auto t : workflow.outer) {
			final_workflow.outer.push_back(t);
		}

		if (!_afs || !_afs->is_open()) {
			std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "File stream closed\n";
			return ASyncData();
		} else {
			return ASyncData(_afs, _size, _offset, final_workflow, _chunk_markers);
		}
	}
}
