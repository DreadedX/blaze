#include "game_asset.h"

namespace BLAZE_NAMESPACE {
	GameAsset::GameAsset(std::string asset_name) : _data(asset_list::find_asset(asset_name)) {}

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
		// @todo Do we need safe_script
		get_lua_state().script(_data.as<std::string>(), environment);
		_loaded = true;

		environment["init"]();
	}


	Language::Language(std::string asset_name) : GameAsset(asset_name) {}

	void Language::post_load() {
		auto size = _data.get_size();
		uint32_t current = 0;

		// @todo Improve this
		while (current < size) {
			auto next = _data[current];
			std::string name = "";
			while (next != 0x0) {
				name += next;
				current++;
				next = _data[current];
			}

			current++;
			next = _data[current];
			std::string value = "";
			while (next != 0x0) {
				value += next;
				current++;
				next = _data[current];
			}
			current++;

			_strings[name] = value;
		}
	}
}
