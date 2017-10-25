#include "asset_manager.h"

namespace BLAZE_NAMESPACE {

	std::list<std::shared_ptr<GameAsset>> asset_manager::_loading_assets;

	void asset_manager::load_assets() {
		_loading_assets.remove_if(GameAsset::finish_if_loaded);
	}

	size_t asset_manager::loading_count() {
		return _loading_assets.size();
	}
}

