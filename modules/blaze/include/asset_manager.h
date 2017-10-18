#pragma once

#include "blaze.h"

#include "game_asset.h"

namespace BLAZE_NAMESPACE::asset_manager {
	namespace _private {
		extern std::list<std::shared_ptr<GameAsset>> loading_assets;
	}

	template <typename T, typename... Args>
	std::shared_ptr<T> new_asset(std::string asset_name, Args... args) {
		static_assert(std::is_base_of<GameAsset, T>(), "T must be derived from GameAsset");
		std::shared_ptr<T> game_asset = std::make_shared<T>(asset_name, args...);
		_private::loading_assets.push_back(game_asset);
		return game_asset;
	}

	void load_assets();
	size_t loading_count();
}
