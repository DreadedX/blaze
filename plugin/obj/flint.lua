lib "tinyobjloader"
	include "vendor/tinyobjloader"

subfile("../../../flint/flint.lua", "flint");
lib "glm"
	include "../../vendor/glm"

shared "plugin_obj"
	path "obj"

	threads()

	dependency("tinyobjloader", "flint_base", "glm")
