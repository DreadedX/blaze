#include "engine.h"
#include "asset_manager.h"
#include "events.h"

#include "bind_flame.h"
#include "bind_blaze.h"

sol::state lua_state;
std::vector<std::shared_ptr<blaze::Script>> scripts;

namespace BLAZE_NAMESPACE {

	void initialize(std::initializer_list<std::string> archives) {
		lua_state.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string);
		flame::lua::bind(lua_state);
		blaze::lua::bind(lua_state);

		for (auto& archive_name : archives) {
			load_archive(archive_name);
		}
	}

	void load_archive(std::string archive_name) {
		#ifdef ANDROID
		std::string filename = "/storage/emulated/0/Android/data/nl.mtgames.blazebootstrap/files/" + archive_name + ".flm";
		#else
		std::string filename = "archives/" + archive_name + ".flm";
		#endif
		try {
			flame::Archive archive(filename);

			blaze::asset_list::add(archive);

			try {
				auto script = blaze::asset_manager::new_asset<blaze::Script>(archive.get_name() + "/Script");
				scripts.push_back(std::move(script));
			} catch (std::exception& e) {
				// @todo We should have a custom exception for this as we now assume an exception means not found
				std::cout << "Archive '" << archive.get_name() << "' does not have a script.\n";
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
}
