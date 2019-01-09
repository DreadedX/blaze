#include "asset_manager.h"

#include "fmt/ostream.h"

namespace BLAZE_NAMESPACE {

	std::list<std::pair<std::shared_ptr<GameAsset>, std::chrono::system_clock::time_point>> asset_manager::_loading_assets;

	void asset_manager::load_assets() {
		// @todo We need to add a verbose debug log option to easily debug this stuff
		// LOG_D("{} assets loading\n", _loading_assets.size());
		// for (auto&& asset : _loading_assets) {
			// LOG_D("\t{}\n", asset->get_name());
		// }

		// _loading_assets.remove_if(GameAsset::finish_if_loaded);
		// @todo Remove if is_loaded() == true
		_loading_assets.remove_if([] (std::pair<std::shared_ptr<GameAsset>, std::chrono::system_clock::time_point> asset) {
			bool loaded = asset.first->is_loaded();
			if (loaded) {
				auto duration = std::chrono::system_clock::now() - asset.second;
				LOG_M("Loading '{}' took {} ms\n", asset.first->get_name(), std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
			}
			return loaded;
		});
	}

	size_t asset_manager::loading_count() {
		return _loading_assets.size();
	}
}

