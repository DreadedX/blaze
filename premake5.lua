require "third_party.premake-androidmk.androidmk"
require "scripts.generate"
require "scripts.functions"

newoption {
	trigger = "platform",
	value = "PLATFORM",
	description = "platform to compile for",
	allowed = {
		{ "linux", "Linux" },
		{ "windows", "Windows" },
		{ "android", "Android" },
		{ "web", "Web browser" },
	}
}

if not _OPTIONS["platform"] then
	_OPTIONS["platform"] = "linux"
end

if _OPTIONS["platform"] == "linux" then
	_ACTION = _ACTION or "gmake"
elseif _OPTIONS["platform"] == "windows" then
	_ACTION = _ACTION or "vs2017"
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

language "C++"
cppdialect "C++17"

workspace "blaze"

	ndkabi "arm64-v8a"
	ndkplatform "android-23"
	ndktoolchainversion "clang"
	ndkstl "c++_shared"

	configurations { "Debug", "Release" }

	if _OPTIONS["platform"] == "windows" then
		systemversion(os.winSdkVersion() .. ".0")
	end

	filter "options:platform=android"
		location("build/android/jni")
	filter "options:not platform=android"
		location("build/" .. _OPTIONS["platform"])
	filter {}

	flags { "MultiProcessorCompile" }
	warnings "Extra"
	rtti "On"
	exceptionhandling "On"
	-- @todo Do we need this
	-- pic "On"

	filter "configurations:Debug"
		defines "DEBUG"
		symbols "On"
		-- @todo This does not work on web
		-- optimize "Debug"

	filter "configurations:Release"
		defines "NDEBUG"
		optimize "On"

	filter {}

-- @todo Figure out which cpp files we really need to compile for each library
project "bigint"
	kind "StaticLib"
	files "third_party/bigint/**"
	removefiles { "third_party/bigint/sample.cc", "third_party/bigint/testsuite.cc" }
	includeBigInt()

project "lua"
	kind "StaticLib"
	files "third_party/lua/**"
	removefiles { "third_party/lua/lua.c" }

project "zlib"
	kind "StaticLib"
	files "third_party/zlib/*.c"
	-- removefiles { "third_party/lua/lua.c" }
	includeZlib()

project "fmt"
	kind "StaticLib"
	files "third_party/fmt/fmt/**.cc"
	includeFmt()

project "generated"
	kind "StaticLib"
	files "modules/generated/src/**"
	includeGenerated()

	dependson "keygen"

project "crypto"
	kind "StaticLib"
	files "modules/crypto/src/**"
	includeCrypto()

project "logger"
	kind "StaticLib"
	files "modules/logger/src/**"
	includeLogger()

project "flame"
	kind "StaticLib"
	files "modules/flame/src/**"
	includeFlame()

project "blaze"
	kind "StaticLib"
	files "modules/blaze/src/**"
	includeBlaze()
	includedirs "modules/blaze/platform/android/include"

if _OPTIONS["platform"] == "android" then
	project "android"
		kind "StaticLib"
		filter{}
		files "modules/blaze/platform/android/src/**"
		includedirs "modules/blaze/platform/android/include"
		includeBlaze()
end

project "lua-bind"
	kind "StaticLib"
	files "modules/lua-bind/src/**"
	includeLuaBind()
	includeFlame()
	includeBlaze()

project "game"
	-- kind "WindowedApp"
	kind "ConsoleApp"
	filter "options:platform=android"
		kind "SharedLib"
	filter {}
	files "game/src/**"
	includedirs "game/include"
	includeBlaze()

-- @note Only build this on desktop platforms
if _OPTIONS["platform"] == "linux" or _OPTIONS["platform"] == "windows" then
	project "keygen"
		kind "ConsoleApp"
		files "tools/keygen/src/**"
		includedirs "tools/keygen/include"
		includeFlame()
		-- @todo Run this if the keys does not yet exist even after keygen has been build
		-- postbuildcommands {"%{cfg.buildtarget.abspath}", "../scripts/generate.lua"}

	project "packager"
		kind "ConsoleApp"
		files "tools/packager/src/**"
		includedirs "tools/packager/include"
		includeGenerated()
		includeLuaBind()
		includeFlame()

		links("stdc++fs")

	project "tests"
		kind "ConsoleApp"
		files "test/**"
		includedirs "third_party/Catch/single_include"
		includeCrypto();

	-- project "content-server"
	-- 	kind "ConsoleApp"
	-- 	files "tools/content-server/src/**"
	-- 	includedirs "tools/content-server/include"
    --
	-- project "testclient"
	-- 	kind "ConsoleApp"
	-- 	files "tools/testclient/src/**"
	-- 	includedirs "tools/testclient/include"
end
