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

	sol::object loader(std::string module_name) {
		// @todo This will block, but there is not really a way around it, unless we maybe make an indirect layer
		try {
			auto data = asset_list::find_asset(module_name);
			return get_lua_state().load(data.as<std::string>());
		} catch (std::exception &e) {
			return sol::make_object(get_lua_state(), "\n\tno asset '" + module_name + '\'');
		}
	}

	void initialize() {
		lua_state.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::table);
		flame::lua::bind(lua_state);
		lua::bind(lua_state);

		// We need to eventually also override print
		lua_state.set_function("log", [](std::string text) {
			log(Level::debug, text + '\n');
		});

		// Add custom loader that allows loading from archives
		sol::table searchers = lua_state["package"]["searchers"];
		searchers.add(&loader);
		
	}

	void load_archive(std::string archive_name) {
		std::string filename = current_platform->get_base_path() + archive_name + ".flm";

		try {
			flame::Archive archive(filename);

			asset_list::add(archive);

			try {
				auto script = asset_manager::new_asset<Script>(archive.get_name() + "/Script");
				scripts.push_back(std::move(script));
			} catch (std::exception& e) {
				// @todo We should have a custom exception for this as we now assume an exception means not found
				log(Level::debug, "Archive '{}' does not gave a script.\n", archive.get_name());
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

	sol::state& get_lua_state() {
		return lua_state;
	}

	std::unique_ptr<platform::Platform>& get_platform() {
		return current_platform;
	}
}
