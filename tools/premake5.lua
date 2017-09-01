project "keygen"
	kind "ConsoleApp"
	location "../build/packager"
	files "keygen/src/**.cpp"
	includedirs { "../modules/flame/include", "../third_party/cryptopp" }
	links { "cryptopp", "flame" }

project "packager"
	kind "ConsoleApp"
	location "../build/packager"
	files "packager/src/**.cpp"
	includedirs { "../modules/flame/include", "../third_party/cryptopp" }
	links { "flame", "cryptopp", "pthread" }
