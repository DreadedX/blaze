#include "engine.h"

#include "bind_flame.h"

sol::state lua;
FLAME_NAMESPACE::AssetList asset_list;

namespace BLAZE_NAMESPACE {
	void initialize(std::initializer_list<std::string> archives) {
		lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string);
		FLAME_NAMESPACE::lua::bind(lua);

		// This is just a function to test if scripts have their own enviroment
		lua.set_function("test", []{
			std::cout << "Hello from C++\n";
		});

		for (auto& archive_name : archives) {
			// @note If we fail to open an archive we will tell the user but continue running as it might not be fatal
			try {
				auto fh = std::make_shared<FLAME_NAMESPACE::FileHandler>(archive_name, std::ios::in);
				FLAME_NAMESPACE::Archive archive(fh);

				asset_list.add(archive);

				// This needs to be run when catching a special exception that returns a list of missing dependencies
				// // We do not add the archive if it is missing a dependecy
				// // @todo Post a event in a central message bus
				// std::cerr << __FILE__ << ':' << __LINE__ << " =>\n\t" << "Missing dependencies:\n";
				// for (auto& missing : missing_dependecies) {
				// 	std::cerr << "\t\t" << missing.first << ':' << missing.second << '\n';
				// }

			} catch (std::exception& e) {
				std::cerr << __FILE__ << ':' << __LINE__ << " =>\n\t" << "Failed to open '" << archive_name << "': " << e.what() << '\n';
				// @todo Post a event in a central message bus
			}
		}
	}

	FLAME_NAMESPACE::AssetList& get_asset_list() {
		return asset_list;
	}

	sol::state& get_lua_state() {
		return lua;
	}
}
