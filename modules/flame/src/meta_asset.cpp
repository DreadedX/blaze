#include "flame/meta_asset.h"
#include "flame/asset_data.h"

#include <iostream>

namespace FLAME_NAMESPACE {
	MetaAsset::MetaAsset(std::string name, std::string filename, size_t version, std::vector<Task> workflow) : _name(name), _fh(std::make_shared<FileHandler>(filename, std::ios::in | std::ios::binary)), _version(version), _offset(0), _base_workflow(workflow) {
		if (!_fh || !_fh->is_open()) {
			throw std::runtime_error("File stream closed");
		}

		auto& fs = _fh->lock();
		fs.seekg(0, std::ios::end);

		_size =  fs.tellg();
		_fh->unlock();
	}

	MetaAsset::MetaAsset(std::string name, std::shared_ptr<FileHandler> fh, size_t version, size_t offset, size_t size, std::vector<Task> workflow) : _name(name), _fh(fh), _version(version), _offset(offset), _size(size), _base_workflow(workflow) {}

	const std::string& MetaAsset::get_name() const {
		return _name;
	}

	size_t MetaAsset::get_version() const {
		return _version;
	}

	AssetData MetaAsset::get_data(bool async, std::vector<Task> workflow) {

		// Construct a new workflow based on the base workflow and given workflow
		std::vector<Task> final_workflow;
		final_workflow.insert(final_workflow.end(), _base_workflow.begin(), _base_workflow.end());
		final_workflow.insert(final_workflow.end(), workflow.begin(), workflow.end());

		return AssetData(_fh, _size, _offset, final_workflow, async);
	}
}
