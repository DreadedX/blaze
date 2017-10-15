require "third_party.premake-androidmk.androidmk"

language "C++"
cppdialect "C++17"

workspace "blaze"
	configurations { "Debug", "Release" }
	platforms { "Linux", "Windows" }
	location "build"
	flags { "MultiProcessorCompile" }
	warnings "Extra"
	rtti "On"
	exceptionhandling "On"
	pic "On"

	ndkabi "armeabi-v7a"
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

	filter "platforms:Windows"
		system "windows"
		architecture "x86_64"


	include "third_party"
	include "modules"
	include "tools"
	include "game"
