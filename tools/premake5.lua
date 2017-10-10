local pkgconfig = require 'pkgconfig'

project "keygen"
	kind "ConsoleApp"
	location "../build/keygen"
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
	location "../build/packager"
	files "packager/src/**.cpp"
	includedirs {
		-- Find better way to link to 5.2
		"/usr/include/lua5.2",
		"packager/include",
		"../modules/flame/include",
		"../modules/lua-flame/include",
		"../third_party/sol2/",
		"../third_party/cryptopp",
	}
	links {
		"lua-flame",
		"flame",
		"cryptopp",
		"lua5.2",
		"z",
		"pthread",
		"dl",
	}
