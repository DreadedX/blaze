-- @todo This is needed in order to support the old packager archives
run_dir ".flint/build/linux/debug/archives"

subfile("modules/iohelper/flint.lua", "iohelper")

lib "bigint"
	src "*third_party/bigint"
	include "third_party/bigint"

lib "lua"
	src("*third_party/lua", "-third_party/lua/lua.c")
	include "third_party/lua"
	include "third_party/headers"

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

	hook(step.PRE_BUILD, template, "modules/generated/src/trusted_key.cpp.tpl", "trusted_key.cpp", function(template, config)
		local key = "test"
		local n = string.gsub(key, ".", function(c)
				return string.format('0x%02X,', string.byte(c))
			end)
		return string.format(template, n)
	end)

	hook(step.PRE_BUILD, template, "modules/generated/src/version.cpp.tpl", "version.cpp", function(template, config)
		local handle = io.popen("git rev-list --count HEAD")
		local build_number = string.gsub(handle:read("a*"), "\n", "")
		handle:close()

		local handle = io.popen("git tag --points-at HEAD")
		local version_tag = string.gsub(handle:read("a*"), "\n", "")
		handle:close()

		local handle = io.popen("git status --porcelain --ignore-submodules=dirty")
		local dirty = handle:read("a*") ~= ""
		handle:close()

		local version_string
		if version_tag == "" or dirty then
			local handle = io.popen("git describe HEAD --always")
			version_string = build_number .. "_" .. string.gsub(handle:read("a*"), "\n", "")
			handle:close()
		else
			version_string = version_tag
		end

		if dirty then
			version_string = version_string .. "_dirty"
		end

		if config.debug then
			version_string = version_string .. "_debug"
		end

		-- @todo Add variable that indicates if we are a debug build
		-- version_string = version_string .. " (debug)"

		return string.format(template, build_number, version_string)
	end)

lib "flame"
	path "modules/flame"
	dependency("zlib", "crypto", "iohelper")

-- @todo This needs to really merge with the respective things
lib "lua-bind"
	path "modules/lua-bind"
	dependency("sol2", "flame")

	include("modules/blaze/include")
	if platform.name == "android" then
		include "modules/blaze/platform/android/include"
	end

lib "blaze"
	path "modules/blaze"
	dependency("lua-bind", "generated")

	-- @todo This should auto happen in flint
	if platform.name == "android" then
		include "modules/blaze/platform/android/include"
		src "*modules/blaze/platform/android/src"
	end

local name = "game"
if platform.name == "android" then
	name = "libgame"
end
executable(name)
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

local packager_path = shared "plugin_packager"
	path "plugin/packager"

	dependency("flint", "flame", "crypto")

	-- We can just use the host platform plugin to generate the archives
	if string.sub(platform.name, 1, string.len("linux")) ~= "linux" then
		optional(true)
	end

-- executable "tests"
-- 	src "test/test.cpp"
--
-- 	include "third_party/Catch/single_include"
--
-- 	dependency "crypto"

run_target "game"

if string.sub(platform.name, 1, string.len("linux")) ~= "linux" then
	-- @todo Figure out something better then hardcoding this
	packager_path = ".flint/build/linux/debug/bin/plugin_packager.so"
end
local packager = plugin(packager_path)

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
