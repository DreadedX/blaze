#include "flame/meta_asset.h"
#include "flame/asset_data.h"

#include <iostream>
#include <fstream>

namespace FLAME_NAMESPACE {
	MetaAsset::MetaAsset(std::string name, std::string filename, size_t version, size_t offset, size_t size, std::vector<Task> workflow) : _filename(filename), _name(name), _version(version), _offset(offset), _size(size), _base_workflow(workflow) {
		// @todo Check if the file exists
		std::fstream fs(filename, std::ios::in | std::ios::binary);

		if (!fs.is_open()) {
			throw std::runtime_error("Failed to open file");
		}

		if (!size) {
			fs.seekg(0, std::ios::end);
			_size = fs.tellg();
		} else {
			_size = size;
		}

		fs.close();
	}

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

		return AssetData(_filename, _size, _offset, final_workflow, async);
	}
}
