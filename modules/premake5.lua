project "flame"
	kind "StaticLib"
	files "flame/src/**.cpp"
	includedirs {
		"flame/include",
		"../third_party/cryptopp",
	}

project "lua-flame"
	kind "StaticLib"
	files "lua-flame/src/**.cpp"
	includedirs {
		"../third_party/lua",
		"lua-flame/include",
		"flame/include",
		"../third_party/sol2/",
		"../third_party/cryptopp",
	}

project "blaze"
	kind "StaticLib"
	files "blaze/src/**.cpp"
	includedirs {
		"../third_party/lua",
		"blaze/include",
		"flame/include",
		"lua-flame/include",
		"../third_party/sol2/",
		"../third_party/cryptopp",
	}
