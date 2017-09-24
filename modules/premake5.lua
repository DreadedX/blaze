project "flame"
	kind "StaticLib"
	location "../build/flame"
	files "flame/src/**.cpp"
	includedirs {
		"flame/include",
		"../third_party/cryptopp",
		"../modules/flame-zlib/include",
	}

project "lua-flame"
	kind "StaticLib"
	location "../build/lua-flame"
	files "lua-flame/src/**.cpp"
	includedirs {
		-- Find better way to link to 5.2
		"/usr/include/lua5.2",
		"lua-flame/include",
		"flame/include",
		"../third_party/sol2/",
		"../third_party/cryptopp",
	}
