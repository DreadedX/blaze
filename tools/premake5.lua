local pkgconfig = require 'pkgconfig'

project "keygen"
	kind "ConsoleApp"
	files "keygen/src/**.cpp"
	includedirs {
		"../modules/flame/include",
		"../third_party/cryptopp",
	}
	links {
		"cryptopp",
		"flame",
	}

project "packager"
	kind "ConsoleApp"
	files "packager/src/**.cpp"
	includedirs {
		"../third_party/lua",
		"packager/include",
		"../modules/flame/include",
		"../modules/lua-flame/include",
		"../third_party/sol2/",
		"../third_party/cryptopp",
	}
	links {
		"lua-flame",
		"flame",
		"lua",
		"cryptopp",
		"z",
		"pthread",
		"dl",
	}
