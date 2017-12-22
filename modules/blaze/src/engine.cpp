#include "logger.h"

#include "engine.h"
#include "asset_manager.h"
#include "events.h"
#include "platform/platform.h"

#include "bind_flame.h"
#include "bind_blaze.h"

#include "enviroment.h"

sol::state lua_state;
std::vector<std::shared_ptr<blaze::Script>> scripts;

std::unique_ptr<blaze::platform::Platform> blaze::current_platform;

namespace BLAZE_NAMESPACE {

	void init() {
		lua_state.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::table);
		flame::lua::bind(lua_state);
		lua::bind(lua_state);
	}

	void load_archive(std::string archive_name) {
		std::string filename = current_platform->get_base_path() + "archives/" + archive_name + ".flm";

		try {
			flame::Archive archive(filename);

			asset_list::add(archive);

			try {
				auto script = asset_manager::new_asset<Script>(archive.get_name() + "/Script");
				scripts.push_back(std::move(script));
			} catch (std::exception& e) {
				// @todo We should have a custom exception for this as we now assume an exception means not found
				LOG_D("Archive '{}' does not have a script.\n", archive.get_name());
			}
		} catch (std::exception& e) {
			event_bus::send(std::make_shared<Error>("Failed to load archive '" + archive_name + "': " + e.what(), __FILE__, __LINE__));
		}
	}

	void update() {
		for (auto& script : scripts) {
			script->update();
		}
	}

	void done() {
		scripts.clear();
		// @todo Make sure we clean everything up properly
	}

	sol::state& get_lua_state() {
		return lua_state;
	}

	std::unique_ptr<platform::Platform>& get_platform() {
		return current_platform;
	}
}
