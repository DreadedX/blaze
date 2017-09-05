language "C++"
cppdialect "C++17"

workspace "blaze"
	configurations { "Debug", "Release" }
	platforms { "Linux", "Windows" }
	location "build"
	flags { "MultiProcessorCompile" }
	warnings "Extra"

	filter "configurations:Debug"
		defines "DEBUG"
		symbols "On"

	filter "configurations:Debug"
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
