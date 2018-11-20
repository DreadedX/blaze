#include "game_asset.h"
#include "game_asset_types.h"

// @todo Split this file

#include "iohelper/memstream.h"

namespace BLAZE_NAMESPACE {
	GameAsset::GameAsset(std::string asset_name) : _data(asset_list::find_asset(asset_name)), _name(asset_name) {}

	bool GameAsset::is_loaded() {
		return _data.is_loaded();
	}

	bool GameAsset::finish_if_loaded(std::shared_ptr<GameAsset> asset) {
		bool loaded = asset->is_loaded();
		if (loaded) {
			asset->post_load();
		}

		return loaded;
	}

	const std::string& GameAsset::get_name() const {
		return _name;
	}

	Script::Script(std::string asset_name) : GameAsset(asset_name), environment(get_lua_state(), sol::create, get_lua_state().globals()) {}

	Script::~Script() {
		// Should always be true as loading_assets will have a valid reference to this object until it is loaded
		if (_loaded) {
			environment["done"]();
		}
	}

	void Script::update() {
		if (_loaded) {
			environment["update"]();
		} else {
			std::cerr << "Asset not loaded!\n";
		}
	}

	void Script::post_load() {
		// @todo Do we need safe_script?
		get_lua_state().script(_data.as<std::string>(), environment);
		_loaded = true;

		environment["init"]();
	}

	Language::Language(std::string asset_name) : GameAsset(asset_name) {}

	void Language::post_load() {
		iohelper::imemstream memstream(_data.as<std::vector<uint8_t>&>());
		_root = lang::parse_file(memstream);
	}
}
