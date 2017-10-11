#include "asset_list.h"

namespace FLAME_NAMESPACE {
	std::vector<Archive> asset_list::_archives;
	std::unordered_map<std::string, MetaAsset> asset_list::_meta_assets;

	AssetData asset_list::find_asset(std::string name) {
		auto meta_asset = _meta_assets.find(name);
		if (meta_asset != _meta_assets.end()) {
			return meta_asset->second.get_data();
		}
		throw std::runtime_error("Can not find asset");
	}

	void asset_list::add(Archive& archive) {
		auto missing = missing_dependecies(archive);
		if (!missing.empty()) {
			/// @todo Maybe make a special exception that returns the list of all missing dependecies
			throw MissingDependencies(missing);
		}

		_archives.push_back(archive);
		for (auto& meta_asset : archive.get_meta_assets()) {
			add(meta_asset);
		}
	}

	void asset_list::add(MetaAsset& meta_asset) {
		// Check if we have already 
		auto existing = _meta_assets.find(meta_asset.get_name());
		if (existing != _meta_assets.end()) {
			if (existing->second.get_version() < meta_asset.get_version()) {
				std::cout << "Replacing asset with newer version: " << meta_asset.get_name() << '\n';
			} else if(existing->second.get_version() > meta_asset.get_version()) {
				std::cout << "Already loaded newer asset: " << meta_asset.get_name() << '\n';
				return;
			} else {
				// @todo Make this a new type of exception so we can specifically catch this one
				throw std::runtime_error("Conflicting asset with same version");
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

	std::vector<std::pair<std::string, uint16_t>> asset_list::missing_dependecies(Archive& archive) {
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
