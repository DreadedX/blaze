#include "bind_flame.h"
#include "helper.h"

// Just to make everything compile
int main() {
	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::io, sol::lib::string, sol::lib::os, sol::lib::table);

	FLAME_NAMESPACE::lua::bind(lua);
	bind(lua);

	lua.script_file("packager.lua");

	return 0;
}

