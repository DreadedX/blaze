#include "asset_list.h"

namespace FLAME_NAMESPACE {
	AssetData AssetList::find_asset(std::string name) {
		auto meta_asset = _meta_assets.find(name);
		if (meta_asset != _meta_assets.end()) {
			return meta_asset->second.get_data();
		}
		std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "Can not find asset\n";
		return AssetData();
	}

	void AssetList::add(Archive& archive) {
		if (archive.is_valid()) {
			_archives.push_back(archive);
		} else {
			std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "Invalid archive\n";
		}
	}

	void AssetList::add(MetaAsset& meta_asset) {
		// Check if we have already 
		auto existing = _meta_assets.find(meta_asset.get_name());
		if (existing != _meta_assets.end()) {
			if (existing->second.get_version() < meta_asset.get_version()) {
				std::cout << "Replacing asset with newer version: " << meta_asset.get_name() << '\n';
			} else if(existing->second.get_version() > meta_asset.get_version()) {
				std::cout << "Already loaded newer asset: " << meta_asset.get_name() << '\n';
				return;
			} else {
				std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "Conflicting assets with same version\n";
				// There is no way we can handle this situation, so we just really on load order
				return;
			}
		}

		_meta_assets[meta_asset.get_name()] = meta_asset;
	}

	bool AssetList::check_dependency(std::pair<std::string, uint16_t> dependency) {
		for (auto &archive : _archives) {
			if (dependency.first == archive.get_name() && dependency.second == archive.get_version()) {
				return true;
			}
		}
		return false;
	}

	void AssetList::load_archives() {
		for (auto& archive : _archives) {
			// Check if the dependecies are loaded
			// @todo This needs testing
			bool found = true;
			for (auto& dependency : archive.get_dependencies()) {
				found = check_dependency(dependency);
				if (!found) {
					std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "Missing dependency: " << dependency.first << ':' << dependency.second << '\n';
				}
				break;
			}
			if (!found) {
				continue;
			}

			for (auto& meta_asset : archive.get_meta_assets()) {
				add(meta_asset);
			}
		}
	}

	void AssetList::debug_list_meta_assets() {
		for (auto& meta_asset : _meta_assets) {
			std::cout << meta_asset.first << '\n';
		}
	}
}
