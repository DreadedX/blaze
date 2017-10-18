#include "engine.h"
#include "events.h"

#include "bind_flame.h"
#include "bind_blaze.h"

sol::state lua_state;

namespace BLAZE_NAMESPACE {

	void initialize(std::initializer_list<std::string> archives) {
		lua_state.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string);
		flame::lua::bind(lua_state);
		blaze::lua::bind(lua_state);

		for (auto& archive_name : archives) {
			// @note If we fail to open an archive we will tell the user but continue running as it might not be fatal
			try {
				auto fh = std::make_shared<flame::FileHandler>(archive_name, std::ios::in);
				flame::Archive archive(fh);

				flame::asset_list::add(archive);

			} catch (flame::MissingDependencies& e) {
				event_bus::send(std::make_shared<MissingDependencies>(archive_name, e.get_missing()));
			} catch (std::exception& e) {
				std::cerr << __FILE__ << ':' << __LINE__ << " =>\n\t" << "Failed to open '" << archive_name << "': " << e.what() << '\n';
				// @todo Post a event in a central message bus
			}
		}
	}

	sol::state& get_lua_state() {
		return lua_state;
	}
}
