-- @todo This is needed in order to support the old packager archives
run_dir(".flint/build/" .. config.platform.target .. "/debug/archives")

plugin "android@Dreaded_X"

subfile("modules/iohelper/flint.lua", "iohelper")

lib "bigint"
	src "*vendor/bigint"
	src "-vendor/bigint/{testsuite,sample}.cc"
	include "vendor/bigint"

lib "lua"
	src("*vendor/lua", "-vendor/lua/lua.c")
	include "vendor/lua"
	include "vendor/headers"

	warnings(false)

lib "zlib"
	src("*vendor/zlib", "-vendor/zlib/{gzlib.c,gzwrite.c,gzread.c}")
	include "vendor/zlib"

	lang "c11"

	warnings(false)

lib "sol2"
	include "vendor/sol2"

	dependency "lua"

lib "glm"
	include "vendor/glm"

lib "vulkan-headers"
	include "vendor/vulkan-headers/include"

lib "glfw"
	include "vendor/glfw/include"
	include "vendor/headers/glfw"

	src "vendor/glfw/src/{context,init,input,monitor,window,vulkan}.c"
	src "vendor/glfw/src/egl_context.c"
	if config.platform.target == "linux" then
		src "vendor/glfw/src/glx_context.c"
		src "vendor/glfw/src/x11_{init,monitor,window}.c"
		src "vendor/glfw/src/posix_{time,tls}.c"
		src "vendor/glfw/src/linux_joystick.c"
		src "vendor/glfw/src/xkb_unicode.c"

		define "_GLFW_X11"
		-- @todo This is needed for strdup to work
		define "_POSIX_C_SOURCE=200809L"

		link("dl", "X11", "Xrandr", "Xinerama", "Xcursor", "vulkan")
	elseif config.platform.target == "windows" then
		src "vendor/glfw/src/wgl_context.c"
		src "vendor/glfw/src/win32_{init,joystick,monitor,time,tls,window}.c"

		define "_GLFW_WIN32"
		link("user32", "kernel32", "gdi32", "shell32", "vulkan-1")

		if config.platform.host == "linux" then
			link_dir "C:/VulkanSDK/1.1.73.0/Lib"
		else
			link_dir "%VK_SDK_PATH%/Lib"
		end
	end

	lang "c11"

	dependency "vulkan-headers"

	warnings(false)

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

lib "blaze"
	path "modules/blaze"
	dependency("flame", "generated", "lang", "sol2")
	dependency "glm"
	if config.platform.target ~= "android" and config.platform.target ~= "web" then
		dependency "glfw"
	end

	if config.platform.target == "web" then
		src "-modules/blaze/src/graphics_backend/vulkan.cpp"
	end

	dependency "vulkan-headers"

	-- @todo This should auto happen in flint
	if config.platform.target == "android" then
		define "VK_USE_PLATFORM_ANDROID_KHR"
		link("log", "android", "EGL", "GLESv1_CM", "vulkan")
	end

subfile("plugin/glsl/flint.lua", "plugin_glsl")
subfile("plugin/image/flint.lua", "plugin_image")
subfile("plugin/obj/flint.lua", "plugin_obj")

meta "plugins"
	dependency("plugin_packager", "plugin_langpack", "plugin_glsl", "plugin_image", "plugin_obj")

-- @todo Allows skipping the second argument
local packager = plugin "packager@blaze"
local langpack_plugin = plugin "langpack@blaze"
local glsl_plugin = plugin "glsl@blaze"
local image_plugin = plugin "image@blaze"
local obj_plugin = plugin "obj@blaze"

if packager and langpack_plugin and glsl_plugin and image_plugin and obj_plugin then
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
	if packager and langpack_plugin and glsl_plugin then
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

	if config.platform.target == "android" then
		flag "-u ANativeActivity_onCreate"
	end

	threads()

git("https://git.mtgames.nl/Dreaded_X/flint", "feature/git", "flint")
subfile(".flint/git/flint/flint.lua", "flint")

local packager_path = shared "plugin_packager"
	path "plugin/packager"

	dependency("flint_base", "flame", "crypto")

	-- We can just use the host platform plugin to generate the archives
	if config.platform.target ~= "linux" then
		optional(true)
	end

	static()

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

	-- @todo We need to include the unistd.h file which comes with win_flex on windows

-- This should actually be a plugin in the future, for now it is just a tool
shared "plugin_langpack"
	path "plugin/lang"
	dependency "plugin_lang-lexer"
	dependency "iohelper"
	dependency "flint_base"

-- executable "tests"
-- 	src "test/test.cpp"
--
-- 	include "vendor/Catch/single_include"
--
-- 	dependency "crypto"

run_target "game"
