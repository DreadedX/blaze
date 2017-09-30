#include "meta_asset.h"
#include "asset_data.h"

#include <iostream>

namespace FLAME_NAMESPACE {
	MetaAsset::MetaAsset(std::string name, std::shared_ptr<FileHandler> fh, uint16_t version, Workflow workflow) : _name(name), _fh(fh), _version(version), _offset(0), _base_workflow(workflow) {
		if (_fh && _fh->is_open()) {
			auto& fs = _fh->lock();
			fs.seekg(0, std::ios::end);
			_size = fs.tellg();
			_fh->unlock();
		} else {
			throw std::runtime_error("File stream closed");
		}
	}

	MetaAsset::MetaAsset(std::string name, std::shared_ptr<FileHandler> fh, uint16_t version, uint32_t offset, uint32_t size, Workflow workflow) : _name(name), _fh(fh), _version(version), _offset(offset), _size(size), _base_workflow(workflow) {}

	const std::string& MetaAsset::get_name() const {
		return _name;
	}

	uint16_t MetaAsset::get_version() const {
		return _version;
	}

	AssetData MetaAsset::get_data(Workflow workflow) {

		// Construct a new workflow based on the base workflow and given workflow
		// @todo Improve this
		Workflow final_workflow;
		for (auto t :_base_workflow.tasks) {
			final_workflow.tasks.push_back(t);
		}
		for (auto t : workflow.tasks) {
			final_workflow.tasks.push_back(t);
		}

		if (!_fh || !_fh->is_open()) {
			throw std::runtime_error("File stream closed");
		} else {
			return AssetData(_fh, _size, _offset, final_workflow);
		}
	}
}
