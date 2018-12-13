#include "logger.h"

#include "asset_list.h"

#include "events.h"
#include "engine.h"

#include "flame/meta_asset.h"

#include <iostream>

namespace BLAZE_NAMESPACE {
	std::vector<flame::Archive> asset_list::_archives;
	std::unordered_map<std::string, flame::MetaAsset> asset_list::_meta_assets;

	flame::AssetData asset_list::find_asset(std::string name, std::function<void(std::vector<uint8_t>)> callback) {
		auto meta_asset = _meta_assets.find(name);
		if (meta_asset != _meta_assets.end()) {
			return meta_asset->second.get_data(get_platform()->has_async_support(), std::vector<flame::MetaAsset::Task>(), callback);
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

		for (auto& meta_asset : archive.get_meta_assets()) {
			add(meta_asset);
		}
	}

	void asset_list::add(flame::MetaAsset& meta_asset) {
		// Check if we have already 
		auto existing = _meta_assets.find(meta_asset.get_name());
		if (existing != _meta_assets.end()) {
			if (existing->second.get_version() < meta_asset.get_version()) {
				// @todo Move this to the event bus
				LOG_D("Replacing asset with newer version: {}\n", meta_asset.get_name());
			} else if(existing->second.get_version() > meta_asset.get_version()) {
				LOG_D("Already loaded newer asset: {}\n", meta_asset.get_name());
				return;
			} else {
				event_bus::send(std::make_shared<Error>("Conflicting asset with same version: '" + meta_asset.get_name() + "' (" + std::to_string(meta_asset.get_version()) + ')', __FILE__, __LINE__));
				return;
			}
		}

		_meta_assets[meta_asset.get_name()] = meta_asset;
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

	void asset_list::debug_list_meta_assets() {
		for (auto& meta_asset : _meta_assets) {
			LOG_D("{}\n", meta_asset.first);
		}
	}
}
