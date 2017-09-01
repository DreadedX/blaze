project "flame"
	kind "StaticLib"
	location "../build/flame"
	files "flame/src/**.cpp"
	includedirs { "flame/include", "../third_party/cryptopp" }
	links "cryptopp"
