#include "game_asset.h"

#include "archive_manager.h"

namespace BLAZE_NAMESPACE {

	GameAsset::GameAsset(std::string asset_name, std::vector<flame::FileHandle::Task> tasks) : _name(asset_name), _data_handle(archive_manager::load_data(asset_name, tasks)) {}

	bool GameAsset::is_loaded() {
		return _data_handle.is_loaded();
	}

	std::string_view GameAsset::get_name() const {
		return _name;
	}
}
