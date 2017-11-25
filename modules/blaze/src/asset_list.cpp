#include "asset_list.h"

#include "events.h"
#include "engine.h"

#include <iostream>

namespace BLAZE_NAMESPACE {
	std::vector<flame::Archive> asset_list::_archives;
	std::unordered_map<std::string, flame::MetaAsset> asset_list::_meta_assets;

	flame::AssetData asset_list::find_asset(std::string name) {
		auto meta_asset = _meta_assets.find(name);
		if (meta_asset != _meta_assets.end()) {
			return meta_asset->second.get_data(get_platform()->has_async_support());
		}
		throw std::runtime_error("Can not find asset: '" + name + '\'');
	}

	void asset_list::add(flame::Archive& archive) {
		auto missing = missing_dependecies(archive);
		if (!missing.empty()) {
			event_bus::send(std::make_shared<MissingDependencies>(archive.get_name(), missing));
			return;
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
				std::cout << "Replacing asset with newer version: " << meta_asset.get_name() << '\n';
			} else if(existing->second.get_version() > meta_asset.get_version()) {
				std::cout << "Already loaded newer asset: " << meta_asset.get_name() << '\n';
				return;
			} else {
				event_bus::send(std::make_shared<Error>("Conflicting asset with same version: '" + meta_asset.get_name() + "' (" + std::to_string(meta_asset.get_version()) + ')', __FILE__, __LINE__));
				return;
			}
		}

		_meta_assets[meta_asset.get_name()] = meta_asset;
	}

	bool asset_list::check_dependency(std::pair<std::string, uint16_t> dependency) {
		for (auto &archive : _archives) {
			if (dependency.first == archive.get_name() && dependency.second == archive.get_version()) {
				return true;
			}
		}
		return false;
	}

	std::vector<std::pair<std::string, uint16_t>> asset_list::missing_dependecies(flame::Archive& archive) {
		std::vector<std::pair<std::string, uint16_t>> missing;
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
			std::cout << meta_asset.first << '\n';
		}
	}
}
