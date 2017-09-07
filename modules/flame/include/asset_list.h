#pragma once

#include "asset.h"
#include "archive.h"
#include "async_data.h"

#include <unordered_map>

namespace blaze::flame {
	class AssetList {
		public:
			ASyncData find_asset(std::string name) {
				auto asset = _assets.find(name);
				if (asset != _assets.end()) {
						return asset->second.get_data();
				}
				std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "Can not find asset\n";
				return ASyncData();
			}

			void debug_list_assets() {
				for (auto& asset : _assets) {
					std::cout << asset.first << '\n';
				}
			}

		private:
			std::unordered_map<std::string, Asset> _assets;

		friend AssetList& operator<<(AssetList& asset_list, Archive& archive);
	};
}
