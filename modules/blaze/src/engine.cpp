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
			auto fh = std::make_shared<FLAME_NAMESPACE::FileHandler>(archive_name, std::ios::in);
			FLAME_NAMESPACE::Archive archive(fh);
			asset_list.add(archive);
		}
		asset_list.load_archives();
	}

	FLAME_NAMESPACE::AssetList& get_asset_list() {
		return asset_list;
	}

	sol::state& get_lua_state() {
		return lua;
	}
}
