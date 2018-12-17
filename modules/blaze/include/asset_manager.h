#pragma once

#include "blaze.h"

#include "game_asset.h"

namespace BLAZE_NAMESPACE {

	class asset_manager {
		public:
			template <typename T, typename... Args>
			static std::shared_ptr<T> new_asset(std::string asset_name, Args... args) {
				static_assert(std::is_base_of<GameAsset, T>(), "T must be derived from GameAsset");
				std::shared_ptr<T> game_asset = std::make_shared<T>(asset_name, args...);
				_loading_assets.push_back(game_asset);
				return game_asset;
			}

			static void load_assets();

			static size_t loading_count();


		private:
			static std::list<std::shared_ptr<GameAsset>> _loading_assets;
	};

}
