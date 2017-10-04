#pragma once

#include "flame.h"

#include "meta_asset.h"
#include "archive.h"
#include "asset_data.h"

#include <unordered_map>

namespace FLAME_NAMESPACE {
	class asset_list {
		public:
			static AssetData find_asset(std::string name);
			static void add(Archive& archive);
			static void add(MetaAsset& meta_asset);
			static bool check_dependency(std::pair<std::string, uint16_t> dependency);
			static std::vector<std::pair<std::string, uint16_t>> missing_dependecies(Archive& archive);
			static void debug_list_meta_assets();

		private:
			// @note Because we have a list of archives, we can actually completely reload all archives and assets
			// @note The GameAsset system in turn can than also reload all of its assets and now we can, on the fly reload all assets that we want
			// @note Debug interface should make it possible to selectively reload GameAssets, it will require a full Archive and Asset reload
			static std::vector<Archive> _archives;
			static std::unordered_map<std::string, MetaAsset> _meta_assets;
	};
}
