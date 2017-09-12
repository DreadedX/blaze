#pragma once

#include "asset.h"
#include "archive.h"
#include "async_data.h"

#include <unordered_map>

namespace blaze::flame {
	class AssetList {
		public:
			ASyncData find_asset(std::string name);
			void add(Archive& archive);
			void add(Asset& asset);
			bool check_dependency(std::pair<std::string, uint16_t> dependency);
			void load_archives();
			void debug_list_assets();

		private:
			// @note Because we have a list of archives, we can actually completely reload all archives and assets
			// @note The GameAsset system in turn can than also reload all of its assets and now we can, on the fly reload all assets that we want
			// @note Debug interface should make it possible to selectively reload GameAssets, it will require a full Archive and Asset reload
			std::vector<Archive> _archives;
			std::unordered_map<std::string, Asset> _assets;
	};
}
