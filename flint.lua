-- @todo This is needed in order to support the old packager archives
run_dir ".flint/build/linux/debug/archives"

plugin "android@Dreaded_X"

subfile("modules/iohelper/flint.lua", "iohelper")

lib "bigint"
	src "*third_party/bigint"
	src "-third_party/bigint/{testsuite,sample}.cc"
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

lib "lang"
	path "modules/lang"
	dependency("iohelper")

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

-- @todo This needs to really merge with the respective things !!!
-- @current
lib "lua-bind"
	path "modules/lua-bind"
	dependency("sol2", "flame", "lang")

	include("modules/blaze/include")
	if config.platform.target == "android" then
		include "modules/blaze/platform/android/include"
	end

lib "blaze"
	path "modules/blaze"
	dependency("lua-bind", "generated", "lang")

	-- @todo This should auto happen in flint
	if config.platform.target == "android" then
		path "modules/blaze/platform/android"
	end
	-- if config.platform.target == "windows" then
	-- 	path "modules/blaze/platform/windows"
	-- end

-- @todo Allows skipping the second argument
local packager = plugin "packager@blaze"

if packager then
	subfile("packager.lua", "")
else
	print "Packager plugin not loaded, skipping building archives"
end

local name = "game"
if config.platform.target == "android" then
	name = "libgame"

	android "game"
		path "modules/android"
		main "nl.mtgames.blaze/.Bootstrap"

		dependency "libgame"
end
executable(name)
	path "game"
	dependency "blaze"

	-- @todo We should add a meta target called game that depends on plugin_packager and that warns the user to build again if packager is not loaded
	-- Or we should just make packagers a seperate project
	if packager then
		dependency "archives"
	end
	-- @todo Does it make sense to always have these as dependencies
	-- During dev it is useful, but will probably get slower in the future
	-- And for release we only have to build this once and it will work on all platforms
	-- Beter idea for in the future ./flint -r game archives
	-- This command build game and the archives and then runs the game
	-- This requires multiple targets and meta targets in flint
	-- Maybe implement run_dependency these are only dependencies when running
	-- run_dependency "base"
	-- Now when building game nothing happend but when running they will get rebuild, unless ofcource -s is set
	
	-- @todo We really need to only build static if we are cross compiling
	if config.platform.target == "windows" and config.platform.host == "linux" then
		static()
	end

	threads()

subfile("../flint/flint.lua", "flint")

local packager_path = shared "plugin_packager"
	path "plugin/packager"

	dependency("flint_base", "flame", "crypto")

	-- We can just use the host platform plugin to generate the archives
	if config.platform.target ~= "linux" then
		optional(true)
	end

	static()

if false then
if config.platform.target == "linux" then
	local parser_lexer = plugin "lexer_parser@Dreaded_X"
	if not parser_lexer then
		print "Plugin parser lexer is needed!"
		os.exit()
	end

	parser "plugin_lang-parser"
		path "plugin/lang"
		dependency "lang"

	lexer "plugin_lang-lexer"
		path "plugin/lang"
		dependency "plugin_lang-parser"

	-- This should actually be a plugin in the future, for now it is just a tool
	executable "plugin_lang"
		path "plugin/lang"
		dependency "plugin_lang-lexer"
		dependency "iohelper"

end

end

-- executable "tests"
-- 	src "test/test.cpp"
--
-- 	include "third_party/Catch/single_include"
--
-- 	dependency "crypto"

run_target "game"
