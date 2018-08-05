-- Load flint plugins
-- plugin "packager"

plugin "plugin_default.so"

run_dir "build/archives"
-- @todo This is only temporary until we have fully ported everything to be based in flint
if target == "packager" then
	run_dir "."
end

lib "bigint"
	src "*third_party/bigint"
	include "third_party/bigint"

lib "lua"
	src("*third_party/lua", "-third_party/lua/lua.c")
	include "third_party/lua"

	-- Generate a hpp file so that we can easily access the library from c++
	os.execute("mkdir -p .flint/generated/lua");
	local header = io.open(".flint/generated/lua/lua.hpp", "w")
	io.output(header)
	io.write('#include "lua.h"\n#include "lualib.h"\n#include "lauxlib.h"')
	io.close()
	include ".flint/generated/lua"

	-- if platform.name == "linux" then
	-- 	link "dl"
	-- end

lib "zlib"
	src("*third_party/zlib", "-third_party/zlib/{gzlib.c,gzwrite.c,gzread.c}")
	include "third_party/zlib"

	lang "c11"

lib "sol2"
	include "third_party/sol2"

	dependency "lua"

subfile("modules/logger/flint.lua", "logger")

lib "parser"
	path "modules/parser"
	dependency "logger"

lib "crypto"
	path "modules/crypto"
	dependency("logger", "bigint")

lib "generated"
	path "modules/generated"
	dependency "crypto"

lib "flame"
	path "modules/flame"
	dependency("zlib", "crypto")

lib "lua-bind"
	path "modules/lua-bind"
	dependency("sol2", "flame")

	include("modules/blaze/include")

lib "blaze"
	path "modules/blaze"
	dependency("lua-bind", "generated")

	include "modules/blaze/platform/android/include"

executable "game"
	path "game"
	dependency "blaze"

executable "packager"
	path "tools/packager"
	dependency("lua-bind", "generated")

	link "stdc++fs"

run_target "game"
