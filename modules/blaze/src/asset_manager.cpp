#include "asset_manager.h"

namespace BLAZE_NAMESPACE::asset_manager {
	std::list<std::shared_ptr<GameAsset>> _private::loading_assets;

	void load_assets() {
		_private::loading_assets.remove_if(finish_load);
	}

	size_t loading_count() {
		return _private::loading_assets.size();
	}
}

