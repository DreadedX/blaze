project "keygen"
	kind "ConsoleApp"
	location "../build/keygen"
	files "keygen/src/**.cpp"
	includedirs { "../modules/flame/include", "../third_party/cryptopp" }
	links { "cryptopp", "flame" }

local pkgconfig = require 'pkgconfig'

project "packager"
	kind "ConsoleApp"
	location "../build/packager"
	files "packager/src/**.cpp"
	includedirs { "packager/include", "../modules/flame/include", "../third_party/cryptopp", "../modules/flame-zlib/include", "../third_party/sol2/single/sol" }
	links { "lua5.2", "flame", "cryptopp", "pthread" }

	pkgconfig.load('lua52')
