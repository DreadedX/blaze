lib "tinyobjloader"
	include "vendor/tinyobjloader"

git("https://git.mtgames.nl/Dreaded_X/flint", "feature/git", "flint")
subfile(".flint/git/flint/flint.lua", "flint")

lib "glm"
	include "../../vendor/glm"

shared "plugin_obj"
	path "obj"

	threads()

	dependency("tinyobjloader", "flint_base", "glm")
