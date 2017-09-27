project "game"
	kind "WindowedApp"
	location "../build/game"
	files "src/**.cpp"
	includedirs {
		-- Find better way to link to 5.2
		"/usr/include/lua5.2",
		"include",
		"../modules/flame/include",
		"../modules/blaze/include",
		"../modules/lua-flame/include",
		"../third_party/sol2/",
		"../third_party/cryptopp",
	}
	links {
		"flame",
		"blaze",
		"lua-flame",
		"cryptopp",
		"lua5.2",
		"z",
		"pthread",
	}
