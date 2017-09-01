project "cryptopp"
	kind "StaticLib"
	location "../build/cryptopp"
	files "cryptopp/*.cpp"
	removefiles {
		"cryptopp/test.cpp",
		"cryptopp/bench1.cpp",
		"cryptopp/bench2.cpp",
		"cryptopp/validat1.cpp",
		"cryptopp/validat2.cpp",
		"cryptopp/validat3.cpp",
		"cryptopp/adhoc.cpp",
		"cryptopp/datatest.cpp",
		"cryptopp/regtest.cpp",
		"cryptopp/fipsalgt.cpp",
		"cryptopp/dlltest.cpp",
		"cryptopp/fipstest.cpp",

		"cryptopp/pch.cpp",
		"cryptopp/simple.cpp",
		"cryptopp/winpipes.cpp",
		"cryptopp/cryptlib_bds.cpp",
	}
	includedirs "cryptopp"
