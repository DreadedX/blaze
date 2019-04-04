#include "logger.h"

#include "archive_manager.h"

#include "events.h"
#include "engine.h"

#include <iostream>

// @todo This entire thing can basically be removed
namespace BLAZE_NAMESPACE {
	std::vector<flame::Archive> archive_manager::_archives;
	flame::Directory* archive_manager::_root = new flame::Directory();
	flame::Directory* archive_manager::_resources = new flame::Directory("resources", _root);

	flame::DataHandle archive_manager::load_data(std::string name, std::vector<flame::FileHandle::Task> tasks) {
		return _root->get_file(name).load_data(get_platform()->has_async_support(), tasks);
	}

	// @todo Reimplement this using the new vfs
	// std::vector<flame::DataHandle> archive_manager::load_all_data(std::string name, std::vector<flame::FileHandle::Task> tasks) {
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

	void archive_manager::add(flame::Archive& archive) {
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
		}

		_archives.push_back(archive);
		_resources->merge_directory(archive.get_directory());

		// for (auto& file_handle : archive.get_file_handles()) {
		// 	add(file_handle);
		// }
	}

	bool archive_manager::check_dependency(flame::Dependency dependency) {
		for (auto &archive : _archives) {
			// @todo Make this statement better
			if ( std::get<0>(dependency) == archive.get_name() && std::get<1>(dependency) <= archive.get_version() && ( (std::get<2>(dependency) == 0) || (std::get<2>(dependency) >= archive.get_version()) ) ) {
				return true;
			}
		}
		return false;
	}

	// @todo We really need to figure out how we want to do dependency stuff...
	// Right now it is specific to blaze, but build into flame
	std::vector<flame::Dependency> archive_manager::missing_dependecies(flame::Archive& archive) {
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

	void archive_manager::debug_list_file_handles() {
		_root->test_tree();
	}
}
