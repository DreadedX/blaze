-- @todo This is needed in order to support the old packager archives
run_dir ".flint/build/linux/debug/archives"

subfile("modules/iohelper/flint.lua", "iohelper")

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

lib "zlib"
	src("*third_party/zlib", "-third_party/zlib/{gzlib.c,gzwrite.c,gzread.c}")
	include "third_party/zlib"

	lang "c11"

lib "sol2"
	include "third_party/sol2"

	dependency "lua"

subfile("modules/logger/flint.lua", "logger")

lib "crypto"
	path "modules/crypto"
	dependency("logger", "bigint", "iohelper")

lib "generated"
	path "modules/generated"
	dependency "crypto"

lib "flame"
	path "modules/flame"
	dependency("zlib", "crypto", "iohelper")

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
	-- @todo Does it make sense to always have these as dependencies
	-- During dev it is useful, but will probably get slower in the future
	-- And for release we only have to build this once and it will work on all platforms
	-- Beter idea for in the future ./flint -r game archives
	-- This command build game and the archives and then runs the game
	-- This requires multiple targets and meta targets in flint
	-- Maybe implement run_dependency these are only dependencies when running
	-- run_dependency "base"
	-- Now when building game nothing happend but when running they will get rebuild, unless ofcource -s is set

subfile("../flint/flint.lua", "flint")

shared "plugin_packager"
	path "plugin/packager"

	dependency("flint", "flame", "crypto")

-- executable "tests"
-- 	src "test/test.cpp"
--
-- 	include "third_party/Catch/single_include"
--
-- 	dependency "crypto"

run_target "game"

-- local packager = plugin ".flint/build/linux/release/bin/plugin_packager.so"
local packager = plugin ".flint/build/linux/debug/bin/plugin_packager.so"

if packager then

	local langpack = require "scripts.langpack"

	meta "archives"
		dependency("base", "my_first_mod")

	archive "my_first_mod"
		author "Dreaded_X"
		description "My first mod!"
		-- key "build/keys/unofficial.priv"
		-- key "keys/official.pem"
		-- compression(flame.Compression.none)
		compression(0)
		version(3)

		script "assets/my_first_mod/script/Script.lua"

		-- @todo This overrides an existing command
		requires "base"
			version_min (1)

		asset "my_first_mod/script/Hello"
			path "assets/my_first_mod/script/Hello.lua"

		asset "base/language/Dutch"
			path "assets/my_first_mod/language/Dutch.lang"
			task (langpack)
			version(10)

	archive "base"
		author "Dreaded_X"
		description "This archive contains the base game"
		-- key "build/keys/test.priv"
		key "keys/official.pem"
		-- compression(flame.Compression.none)
		compression(0)
		version(1)

		script "assets/base/script/Script.lua"

		asset "base/language/Dutch"
			path "assets/base/language/Dutch.lang"
			task (langpack)

		asset "base/language/English"
			path "assets/base/language/English.lang"
			task (langpack)

else
	print "Packager plugin not loaded, skipping building archives"
end
