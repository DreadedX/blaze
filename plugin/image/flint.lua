lib "stb"
	include "vendor/stb"

git("https://git.mtgames.nl/Dreaded_X/flint", "feature/git", "flint")
subfile(".flint/git/flint/flint.lua", "flint")

git("https://git.mtgames.nl/Blaze/iohelper", "master", "iohelper")
subfile(".flint/git/iohelper/flint.lua", "iohelper")

shared "plugin_image"
	path "image"

	threads()

	dependency("stb", "flint_base", "iohelper")
