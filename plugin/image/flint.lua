lib "stb"
	include "vendor/stb"

subfile("../../../flint/flint.lua", "flint");

subfile("../../../iohelper/flint.lua", "iohelper");

shared "plugin_image"
	path "image"

	threads()

	dependency("stb", "flint_base", "iohelper")
