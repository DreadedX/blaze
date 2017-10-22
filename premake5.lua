require "third_party.premake-androidmk.androidmk"
require "scripts.generate"
require "scripts.functions"

defaultAction("linux", "gmake")

language "C++"
cppdialect "C++17"

workspace "blaze"
	configurations { "Debug", "Release" }
	platforms { "Linux" }
	if _ACTION == "androidmk" then
		location "android/jni"
	else
		location "build"
	end
	flags { "MultiProcessorCompile" }
	warnings "Extra"
	rtti "On"
	exceptionhandling "On"
	pic "On"

	ndkabi "arm64-v8a"
	ndkplatform "android-23"
	ndktoolchainversion "clang"
	ndkstl "c++_shared"

	filter "configurations:Debug"
		defines "DEBUG"
		symbols "On"
		optimize "Debug"

	filter "configurations:Release"
		defines "NDEBUG"
		optimize "On"

	filter "platforms:Linux"
		system "linux"
		architecture "x86_64"

project "cryptopp"
	kind "StaticLib"
	cppdialect "C++14"
	files "third_party/cryptopp/*.cpp"
	removefiles { "third_party/cryptopp/test.cpp", "third_party/cryptopp/bench1.cpp", "third_party/cryptopp/bench2.cpp", "third_party/cryptopp/validat1.cpp", "third_party/cryptopp/validat2.cpp", "third_party/cryptopp/validat3.cpp", "third_party/cryptopp/adhoc.cpp", "third_party/cryptopp/datatest.cpp", "third_party/cryptopp/regtest.cpp", "third_party/cryptopp/fipsalgt.cpp", "third_party/cryptopp/dlltest.cpp", "third_party/cryptopp/fipstest.cpp", "third_party/cryptopp/pch.cpp", "third_party/cryptopp/simple.cpp", "third_party/cryptopp/winpipes.cpp", "third_party/cryptopp/cryptlib_bds.cpp" }
	includeCryptoPP()

project "lua"
	kind "StaticLib"
	files "third_party/lua/**"

project "generated"
	kind "StaticLib"
	files "modules/generated/src/**"
	includeGenerated()

	dependson "keygen"

project "flame"
	kind "StaticLib"
	files "modules/flame/src/**"
	includeFlame()

project "blaze"
	kind "StaticLib"
	files "modules/blaze/src/**"
	includeBlaze()

project "lua-bind"
	kind "StaticLib"
	files "modules/lua-bind/src/**"
	includeLuaBind()
	includeFlame()
	includeBlaze()

project "keygen"
	-- removeplatforms { "Android" }
	kind "ConsoleApp"
	files "tools/keygen/src/**"
	includedirs "tools/keygen/include"
	includeFlame()

	-- @todo Run this if the keys does not yet exist even after keygen has been build
	postbuildcommands {"%{cfg.buildtarget.abspath}", "../scripts/generate.lua"}

project "packager"
	kind "ConsoleApp"
	files "tools/packager/src/**"
	includedirs "tools/packager/include"
	includeGenerated()
	includeLuaBind()
	includeFlame()
	-- @todo This depends on the platform
	links "dl"

project "game"
	kind "WindowedApp"
	files "game/src/**"
	includedirs "game/include"
	includeBlaze()
	includePlatform()
