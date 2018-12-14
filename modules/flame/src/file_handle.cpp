#include "flame/file_handle.h"

#include <iostream>
#include <fstream>

namespace FLAME_NAMESPACE {
	FileHandle::FileHandle(std::string name, size_t version, internal::FileInfo file_info, std::vector<Task> workflow) : _file_info(file_info), _name(name), _version(version), _base_workflow(workflow) {}

	const std::string& FileHandle::get_name() const {
		return _name;
	}

	size_t FileHandle::get_version() const {
		return _version;
	}

	std::vector<uint8_t> FileHandle::async_load(std::vector<Task> workflow) {
		std::vector<Task> final_workflow;
		final_workflow.insert(final_workflow.end(), _base_workflow.begin(), _base_workflow.end());
		final_workflow.insert(final_workflow.end(), workflow.begin(), workflow.end());

		std::vector<uint8_t> data(_file_info.size);

		std::fstream fs(_file_info.filename, std::ios::in | std::ios::binary);
		if (!fs.is_open()) {
			throw std::runtime_error("ASYNC: Failed to open file");
		}

		fs.seekg(_file_info.offset);
		fs.read(reinterpret_cast<char*>(data.data()), _file_info.size);

		for (auto& t : final_workflow) {
			data = t(std::move(data));
		}

		return data;
	}

	DataLoader FileHandle::get_data(bool async, std::vector<Task> workflow) {
		std::launch policy = async ? std::launch::async : std::launch::deferred;
		return DataLoader(std::async(policy, std::bind(&FileHandle::async_load, this, std::placeholders::_1), workflow), async);
	}
}
