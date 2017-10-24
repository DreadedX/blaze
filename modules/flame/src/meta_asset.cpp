#include "meta_asset.h"
#include "asset_data.h"

#include <iostream>

namespace FLAME_NAMESPACE {
	MetaAsset::MetaAsset(std::string name, std::string filename, uint16_t version, std::vector<Task> workflow) : _name(name), _fh(std::make_shared<FileHandler>(filename, std::ios::in)), _version(version), _offset(0), _base_workflow(workflow) {
		if (!_fh || !_fh->is_open()) {
			throw std::runtime_error("File stream closed");
		}

		auto& fs = _fh->lock();
		fs.seekg(0, std::ios::end);
		_size = fs.tellg();
		_fh->unlock();
	}

	MetaAsset::MetaAsset(std::string name, std::shared_ptr<FileHandler> fh, uint16_t version, uint32_t offset, uint32_t size, std::vector<Task> workflow) : _name(name), _fh(fh), _version(version), _offset(offset), _size(size), _base_workflow(workflow) {}

	const std::string& MetaAsset::get_name() const {
		return _name;
	}

	uint16_t MetaAsset::get_version() const {
		return _version;
	}

	AssetData MetaAsset::get_data(std::vector<Task> workflow) {

		// Construct a new workflow based on the base workflow and given workflow
		// @todo Improve this
		std::vector<Task> final_workflow;
		for (auto t :_base_workflow) {
			final_workflow.push_back(t);
		}
		for (auto t : workflow) {
			final_workflow.push_back(t);
		}

		return AssetData(_fh, _size, _offset, final_workflow);
	}
}
