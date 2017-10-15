project "game"
	kind "WindowedApp"
	files "src/**.cpp"
	includedirs {
		"../third_party/lua",
		"include",
		"../modules/flame/include",
		"../modules/blaze/include",
		"../modules/lua-flame/include",
		"../third_party/sol2/",
		"../third_party/cryptopp",
	}
	links {
		"blaze",
		"lua-flame",
		"flame",
		"lua",
		"cryptopp",
		"z",
		"pthread",
	}
