#include "game_asset.h"

#include "asset_list.h"

namespace BLAZE_NAMESPACE {

	GameAsset::GameAsset(std::string asset_name, std::vector<flame::FileHandle::Task> tasks) : _name(asset_name), _data_handle(asset_list::load_data(asset_name, tasks)) {}

	bool GameAsset::is_loaded() {
		return _data_handle.is_loaded();
	}

	std::string_view GameAsset::get_name() const {
		return _name;
	}
}
