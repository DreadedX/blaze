#include "game_asset.h"
#include "game_asset_types.h"

// @todo Split this file

#include "iohelper/memstream.h"

// @todo This is just for the thread id stuff
#include <fmt/ostream.h>

namespace BLAZE_NAMESPACE {

	GameAssetBase::GameAssetBase(std::string asset_name) : _name(asset_name) {}

	std::string_view GameAssetBase::get_name() const {
		return _name;
	}

	GameAssetLoaded::GameAssetLoaded(std::string asset_name, std::vector<flame::MetaAsset::Task> tasks) : GameAssetBase(asset_name), _data(asset_list::find_asset(asset_name, tasks)) {}

	bool GameAssetLoaded::is_loaded() {
		return _data.is_loaded();
	}

	Script::Script(std::string asset_name) : GameAssetLoaded(asset_name), environment(get_lua_state(), sol::create, get_lua_state().globals()) {}

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
		bool loaded = GameAssetLoaded::is_loaded();
		if (loaded && !_loaded) {
			LOG_D("Calling lua init function ({})\n", get_name());
			get_lua_state().script(_data.as<std::string>(), environment);
			_loaded = true;

			environment["init"]();
		}

		return loaded;
	}

	Language::Language(std::string asset_name) : GameAssetLoaded(asset_name, {std::bind(&Language::load, this, std::placeholders::_1)}) {}

	// Kind of abuse the flame task system to run callbacks on other threads
	std::vector<uint8_t> Language::load(std::vector<uint8_t> data) {
		LOG_D("Thread id: {} ({})\n", std::this_thread::get_id(), get_name());

		iohelper::imemstream memstream(data);
		_root = lang::parse_file(memstream);

		return data;
	}
}
