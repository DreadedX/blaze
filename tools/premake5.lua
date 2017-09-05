project "keygen"
	kind "ConsoleApp"
	location "../build/keygen"
	files "keygen/src/**.cpp"
	includedirs { "../modules/flame/include", "../third_party/cryptopp" }
	links { "cryptopp", "flame" }

local pkgconfig = require 'pkgconfig'

project "packager"
	kind "WindowedApp"
	location "../build/packager"
	files "packager/src/**.cpp"
	includedirs { "packager/include", "../modules/flame/include", "../third_party/cryptopp"}
	links { "flame", "cryptopp", "pthread", "gtk+-3.0" }

	-- pkgconfig.load('gtkmm-3.0')
