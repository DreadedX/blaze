#include "bind_flame.h"
#include "helper.h"

#include "logger.h"

// Just to make everything compile
int main(int argc, char* argv[]) {

	logger::add(logger::std_logger);

	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::io, sol::lib::string, sol::lib::os, sol::lib::table, sol::lib::math);

	FLAME_NAMESPACE::lua::bind(lua);
	bind(lua);

	lua.script_file("scripts/builder.lua");

	lua.script_file("packager.lua");

	auto system = lua.require_file("make", "scripts/make.lua", false);
	sol::function generate = lua["generate"];
	sol::function package = lua["package"];
	sol::function build = lua["build"];
	sol::function run = lua["run"];

	if (argc >= 2 && !std::string("generate").compare(argv[1])) {
		generate(system);
	} else if (argc >= 2 && !std::string("package").compare(argv[1])) {
		package();
	} else if (argc >= 2 && !std::string("build").compare(argv[1])) {
		build(system);
	} else if (argc >= 2 && !std::string("run").compare(argv[1])) {
		if (argc >= 3) {
			run(std::string(argv[2]));
		} else {
			run();
		}
	} else {
		LOG_M("Invalid thing\n");
	}

	return 0;
}

