#include "logger.h"

#include "asset_list.h"

#include "events.h"
#include "engine.h"

#include <iostream>

namespace BLAZE_NAMESPACE {
	std::vector<flame::Archive> asset_list::_archives;
	std::unordered_map<std::string, flame::FileHandle> asset_list::_file_handles;

	flame::DataHandle asset_list::load_data(std::string name, std::vector<flame::FileHandle::Task> tasks) {
		auto file_handle = _file_handles.find(name);
		if (file_handle != _file_handles.end()) {
			return file_handle->second.load_data(get_platform()->has_async_support(), tasks);
		}
		throw std::runtime_error("Can not find asset: '" + name + '\'');
	}

	void asset_list::add(flame::Archive& archive) {
		auto missing = missing_dependecies(archive);
		if (!missing.empty()) {
			event_bus::send(std::make_shared<MissingDependencies>(archive.get_name(), missing));
			// @todo How exactly are we going to handle this
			throw std::runtime_error("Missing dependencies");
			// return;
		}

		_archives.push_back(archive);

		for (auto& file_handle : archive.get_file_handles()) {
			add(file_handle);
		}
	}

	void asset_list::add(flame::FileHandle& file_handle) {
		// Check if we have already 
		auto existing = _file_handles.find(file_handle.get_name());
		if (existing != _file_handles.end()) {
			if (existing->second.get_version() < file_handle.get_version()) {
				// @todo Move this to the event bus
				LOG_D("Replacing asset with newer version: {}\n", file_handle.get_name());
				existing->second = file_handle;
			} else if(existing->second.get_version() > file_handle.get_version()) {
				LOG_D("Already loaded newer asset: {}\n", file_handle.get_name());
			} else {
				event_bus::send(std::make_shared<Error>("Conflicting asset with same version: '" + file_handle.get_name() + "' (" + std::to_string(file_handle.get_version()) + ')', __FILE__, __LINE__));
			}
			return;
		}

		_file_handles.insert({file_handle.get_name(), file_handle});
	}

	bool asset_list::check_dependency(flame::Dependency dependency) {
		for (auto &archive : _archives) {
			// @todo Make this statement better
			if ( std::get<0>(dependency) == archive.get_name() && std::get<1>(dependency) <= archive.get_version() && ( (std::get<2>(dependency) == 0) || (std::get<2>(dependency) >= archive.get_version()) ) ) {
				return true;
			}
		}
		return false;
	}

	std::vector<flame::Dependency> asset_list::missing_dependecies(flame::Archive& archive) {
		std::vector<flame::Dependency> missing;
		// Check if the dependecies are loaded
		// @todo This needs testing
		bool found = true;
		for (auto& dependency : archive.get_dependencies()) {
			found = check_dependency(dependency);
			if (!found) {
				missing.push_back(dependency);
			}
		}

		return missing;
	}

	void asset_list::debug_list_file_handles() {
		for (auto& file_handle : _file_handles) {
			LOG_D("{}\n", file_handle.first);
		}
	}
}
