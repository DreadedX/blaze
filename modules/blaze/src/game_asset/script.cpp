#include "game_asset/script.h"

#include "engine.h"

namespace BLAZE_NAMESPACE {
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

	bool Script::is_loaded() {
		bool loaded = GameAsset::is_loaded();
		if (loaded && !_loaded) {
			LOG_D("Calling lua init function ({})\n", get_name());
			get_lua_state().script(_data_handle.get_as<std::string>(), environment);
			_loaded = true;

			environment["init"]();
		}

		return loaded;
	}
}
