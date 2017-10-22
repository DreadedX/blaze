#include "engine.h"
#include "asset_manager.h"
#include "events.h"

#include "bind_flame.h"
#include "bind_blaze.h"

sol::state lua_state;
std::vector<std::shared_ptr<blaze::Script>> scripts;

namespace BLAZE_NAMESPACE {

	// @todo Make this public eventually (e.g. for a mod loader)
	void load_archive(std::string archive_name) {
		// @note If we fail to open an archive we will tell the user but continue running as it might not be fatal
		try {
			auto fh = std::make_shared<flame::FileHandler>(archive_name, std::ios::in);
			flame::Archive archive(fh);

			flame::asset_list::add(archive);

			try {
				auto script = blaze::asset_manager::new_asset<blaze::Script>(archive.get_name() + "/Script");
				scripts.push_back(std::move(script));
			} catch (std::exception& e) {
				std::cout << "Archive '" << archive.get_name() << "' does not have a script.\n";
			}

		} catch (flame::MissingDependencies& e) {
			event_bus::send(std::make_shared<MissingDependencies>(archive_name, e.get_missing()));
		} catch (std::exception& e) {
			std::cerr << __FILE__ << ':' << __LINE__ << " =>\n\t" << "Failed to open '" << archive_name << "': " << e.what() << '\n';
			// @todo Post a event in a central message bus
		}
	}

	void initialize(std::initializer_list<std::string> archives) {
		lua_state.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string);
		flame::lua::bind(lua_state);
		blaze::lua::bind(lua_state);

		for (auto& archive_name : archives) {
			load_archive(archive_name);
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
