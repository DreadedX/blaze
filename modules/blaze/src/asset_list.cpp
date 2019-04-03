#include "logger.h"

#include "asset_list.h"

#include "events.h"
#include "engine.h"

#include <iostream>

// @todo This entire thing can basically be removed
namespace BLAZE_NAMESPACE {
	std::vector<flame::Archive> asset_list::_archives;
	flame::Directory* asset_list::_root = new flame::Directory();
	flame::Directory* asset_list::_resources = new flame::Directory("resources", _root);

	flame::DataHandle asset_list::load_data(std::string name, std::vector<flame::FileHandle::Task> tasks) {
		return _root->get_file(name).load_data(get_platform()->has_async_support(), tasks);
	}

	// @todo Reimplement this using the new vfs
	// std::vector<flame::DataHandle> asset_list::load_all_data(std::string name, std::vector<flame::FileHandle::Task> tasks) {
	// 	std::vector<flame::DataHandle> data_handles;
    //
	// 	auto existing = _file_handles.find(name);
	// 	if (existing != _file_handles.end()) {
	// 		for (auto& file_handle : existing->second) {
	// 			data_handles.push_back(file_handle.load_data(get_platform()->has_async_support(), tasks));
	// 		}
	// 	} else {
	// 		throw std::runtime_error("Can not find asset: '" + name + '\'');
	// 	}
    //
	// 	return data_handles;
	// }

	void asset_list::add(flame::Archive& archive) {
		static bool a = false;
		if (!a) {
			_root->add_directory(_resources);
			a = true;
		}
		auto missing = missing_dependecies(archive);
		if (!missing.empty()) {
			event_bus::send(std::make_shared<MissingDependencies>(archive.get_name(), missing));
			// @todo How exactly are we going to handle this
			throw std::runtime_error("Missing dependencies");
			// return;
		}

		_archives.push_back(archive);

		// for (auto& file_handle : archive.get_file_handles()) {
		// 	add(file_handle);
		// }
	}

	// void asset_list::add(flame::FileHandle& file_handle) {
	// 	// Check if we have already 
	// 	auto existing = _file_handles.find(file_handle.get_name());
	// 	if (existing != _file_handles.end()) {
	// 		if (existing->second.back().get_version() < file_handle.get_version()) {
	// 			// @todo Move this to the event bus
	// 			LOG_D("Adding newer asset: {}\n", file_handle.get_name());
	// 			existing->second.push_back(file_handle);
	// 		} else if(existing->second.back().get_version() > file_handle.get_version()) {
	// 			LOG_D("Storing older asset: {}\n", file_handle.get_name());
	// 			// @todo Make sure this actually works correctly
	// 			size_t i = existing->second.size();
	// 			LOG_D("Size {}\n", i);
	// 			for (; i--;) {
	// 				LOG_D("i = {}\n", i);
	// 				// If same version last loaded is higher in the list (inverted list)
	// 				if (existing->second[i].get_version() <= file_handle.get_version()) {
	// 					break;
	// 				}
	// 			}
	// 			LOG_D("Inserting in {}\n", i+1);
	// 			existing->second.insert(existing->second.begin() + i + 1, file_handle);
	// 		} else {
	// 			event_bus::send(std::make_shared<Error>("Conflicting asset with same version: '" + file_handle.get_name() + "' (" + std::to_string(file_handle.get_version()) + ')', __FILE__, __LINE__));
	// 			// @todo Is this how we want to handle conflicts?
	// 			// This will be based on load order
	// 			existing->second.push_back(file_handle);
	// 		}
	// 		return;
	// 	}
    //
	// 	LOG_D("New asset: {}\n", file_handle.get_name());
    //
	// 	_file_handles.insert({file_handle.get_name(), {file_handle}});
	// }

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
		_root->test_tree();
	}
}
