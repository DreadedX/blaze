#include "asset_list.h"

namespace blaze::flame {
	ASyncData AssetList::find_asset(std::string name) {
		auto asset = _assets.find(name);
		if (asset != _assets.end()) {
			return asset->second.get_data();
		}
		std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "Can not find asset\n";
		return ASyncData();
	}

	void AssetList::add(Archive& archive) {
		if (archive.is_valid()) {
			_archives.push_back(archive);
		} else {
			std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "Invalid archive\n";
		}
	}

	void AssetList::add(Asset& asset) {
		// Check if we have already 
		auto existing = _assets.find(asset.get_name());
		if (existing != _assets.end()) {
			if (existing->second.get_version() < asset.get_version()) {
				std::cout << "Replacing asset with newer version: " << asset.get_name() << '\n';
			} else if(existing->second.get_version() > asset.get_version()) {
				std::cout << "Already loaded newer asset: " << asset.get_name() << '\n';
				return;
			} else {
				std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "Conflicting assets with same version\n";
				// There is no way we can handle this situation, so we just really on load order
				return;
			}
		}

		_assets[asset.get_name()] = asset;
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

			for (auto& asset : archive.get_assets()) {
				add(asset);
			}
		}
	}

	void AssetList::debug_list_assets() {
		for (auto& asset : _assets) {
			std::cout << asset.first << '\n';
		}
	}
}
