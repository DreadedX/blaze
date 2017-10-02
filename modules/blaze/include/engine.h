#pragma once

#include "blaze.h"

#include "asset_list.h"
#include "events.h"

#include "sol/state.hpp"

#include <initializer_list>
#include <string>

// @todo Refactor the shit out of this, most of this is just the first iteration
namespace BLAZE_NAMESPACE {
	void initialize(std::initializer_list<std::string> archives);
	FLAME_NAMESPACE::AssetList& get_asset_list();
	sol::state& get_lua_state();
	EventBus& get_event_bus();

	class GameAsset {
		public:
			virtual ~GameAsset() {}
	};

	class LuaScript : public GameAsset {
		public:
			LuaScript(std::string asset_name) : environment(get_lua_state(), sol::create, get_lua_state().globals()) {
				auto script = get_asset_list().find_asset(asset_name);
				code = std::string(reinterpret_cast<const char*>(script.data()));
			}

			void run() {
				// @todo This need to run in it's own sandbox, otherwise it can access values stored in other scripts, but storage per script should be persistent
				get_lua_state().safe_script(code, environment);
				std::cout << "Memory usage: " << get_lua_state().memory_used() << '\n';
			}

		private:
			std::string code;
			// Each script gets its own enviroment
			sol::environment environment;
	};
}
