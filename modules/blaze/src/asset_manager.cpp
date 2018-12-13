#include "asset_manager.h"

namespace BLAZE_NAMESPACE {

	std::list<std::shared_ptr<GameAssetBase>> asset_manager::_loading_assets;

	void asset_manager::load_assets() {
		// @todo We need to add a verbose debug log option to easily debug this stuff
		LOG_D("{} assets loading\n", _loading_assets.size());
		for (auto&& asset : _loading_assets) {
			LOG_D("\t{}\n", asset->get_name());
		}

		// _loading_assets.remove_if(GameAsset::finish_if_loaded);
		// @todo Remove if is_loaded() == true
		_loading_assets.remove_if([] (std::shared_ptr<GameAssetBase> asset) {
			return asset->is_loaded();
		});
	}

	size_t asset_manager::loading_count() {
		return _loading_assets.size();
	}
}

