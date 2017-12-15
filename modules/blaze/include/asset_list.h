#pragma once

#include "blaze.h"

#include "meta_asset.h"
#include "archive.h"
#include "asset_data.h"

#include <unordered_map>

namespace BLAZE_NAMESPACE {

	class asset_list {
		public:
			static flame::AssetData find_asset(std::string name);
			static void add(flame::Archive& archive);
			static void add(flame::MetaAsset& meta_asset);
			static bool check_dependency(flame::Dependency dependency);
			static std::vector<flame::Dependency> missing_dependecies(flame::Archive& archive);
			static void debug_list_meta_assets();

		private:
			// @note Because we have a list of archives, we can actually completely reload all archives and assets
			// @note The GameAsset system in turn can than also reload all of its assets and now we can, on the fly reload all assets that we want
			// @note Debug interface should make it possible to selectively reload GameAssets, it will require a full Archive and Asset reload
			static std::vector<flame::Archive> _archives;
			static std::unordered_map<std::string, flame::MetaAsset> _meta_assets;
	};
}
