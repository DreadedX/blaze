require "third_party.premake-androidmk.androidmk"
require "scripts.generate"
require "scripts.functions"

newoption {
	trigger = "platform",
	value = "PLATFORM",
	description = "platform to compile for",
	allowed = {
		{ "linux", "Linux" },
		{ "android", "Android" },
		{ "web", "Web browser" },
	}
}

if not _OPTIONS["platform"] then
	_OPTIONS["platform"] = "linux"
end

if _OPTIONS["platform"] == "linux" then
	_ACTION = _ACTION or "gmake"
elseif _OPTIONS["platform"] == "android" then
	_ACTION = _ACTION or "androidmk"
elseif _OPTIONS["platform"] == "web" then
	_ACTION = _ACTION or "gmake"
end

-- newaction {
-- 	trigger = "run",
-- 	description = "Run the software",
-- 	execute = function()
-- 		print "Hello world!"
-- 	end
-- }

workspace "blaze"
	language "C++"
	cppdialect "C++17"

	ndkabi "arm64-v8a"
	ndkplatform "android-23"
	ndktoolchainversion "clang"
	ndkstl "c++_shared"

	configurations { "Debug", "Release" }

	filter "options:platform=android"
		location("build/android/jni")
	filter "options:not platform=android"
		location("build/" .. _OPTIONS["platform"])
	filter {}

	flags { "MultiProcessorCompile" }
	warnings "Extra"
	rtti "On"
	exceptionhandling "On"
	pic "On"

	filter "configurations:Debug"
		defines "DEBUG"
		symbols "On"
		-- @todo This does not work on web
		-- optimize "Debug"

	filter "configurations:Release"
		defines "NDEBUG"
		optimize "On"

	filter {}

-- @todo For some reason android ignores removefiles
-- @todo Figure out which cpp files we really need to compile for each library
project "bigint"
	kind "StaticLib"
	files "third_party/bigint/**"
	removefiles { "sample.cc", "testsuite.cc" }
	includeBigInt()

project "lua"
	kind "StaticLib"
	files "third_party/lua/**"
	removefiles { "lua.c" }

project "generated"
	kind "StaticLib"
	files "modules/generated/src/**"
	includeGenerated()

	dependson "keygen"

project "crypto"
	kind "StaticLib"
	files "modules/crypto/src/**"
	includeCrypto()

project "flame"
	kind "StaticLib"
	files "modules/flame/src/**"
	includeFlame()

project "blaze"
	kind "StaticLib"
	files "modules/blaze/src/**"
	includeBlaze()
	includedirs "modules/blaze/platform/android/include"

-- @todo Kinda weird to have to use utility to prevent a project from building
project "android"
	kind "Utility"
	filter "options:platform=android"
		kind "StaticLib"
	filter{}
	files "modules/blaze/platform/android/src/**"
	includedirs "modules/blaze/platform/android/include"
	includeBlaze()

project "lua-bind"
	kind "StaticLib"
	files "modules/lua-bind/src/**"
	includeLuaBind()
	includeFlame()
	includeBlaze()

project "game"
	kind "WindowedApp"
	filter "options:platform=android"
		kind "SharedLib"
	filter {}
	files "game/src/**"
	includedirs "game/include"
	includeBlaze()

project "keygen"
	kind "ConsoleApp"
	filter "options:platform=android"
		kind "SharedLib"
	filter {}
	files "tools/keygen/src/**"
	includedirs "tools/keygen/include"
	includeFlame()
	-- @todo Run this if the keys does not yet exist even after keygen has been build
	-- postbuildcommands {"%{cfg.buildtarget.abspath}", "../scripts/generate.lua"}

project "packager"
	kind "ConsoleApp"
	filter "options:platform=android"
		kind "SharedLib"
	filter {}
	files "tools/packager/src/**"
	includedirs "tools/packager/include"
	includeGenerated()
	includeLuaBind()
	includeFlame()
	-- @todo This depends on the platform
	links "dl"

project "tests"
	kind "ConsoleApp"
	filter "options:platform=android"
		kind "SharedLib"
	filter {}
	files "test/**"
	includedirs "third_party/Catch/single_include"
	includeCrypto();
