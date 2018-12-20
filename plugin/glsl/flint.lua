lib "spirv-headers"
	include "vendor/spirv-headers/include"

lib "spirv-tools"
	src "**vendor/spirv-tools/source"
	include "vendor/spirv-tools/include"
	include "vendor/spirv-tools"
	include "vendor/headers"

	dependency "spirv-headers"

lib "libshaderc_util"
	src "vendor/shaderc/libshaderc_util/src/{compiler,file_finder,io,message,resources,shader_stage,spirv_tools_wrapper,version_profile}.cc"
	include "vendor/shaderc/libshaderc_util/include"

	dependency "spirv-tools"

lib "glslang"
	src "**vendor/glslang/glslang/GenericCodeGen"
	src "**vendor/glslang/glslang/MachineIndependent"
	src "**vendor/glslang/SPIRV"
	if config.platform.target == "windows" then
		src "**vendor/glslang/glslang/OSDependent/Windows"
	else
		src "**vendor/glslang/glslang/OSDependent/Unix"
	end

	src "**vendor/glslang/OGLCompilersDLL"

	include "vendor/glslang/SPIRV"
	include "vendor/glslang/glslang/Include"
	include "vendor/glslang/glslang/Public"

lib "libshaderc"
	src "vendor/shaderc/libshaderc/src/shaderc.cc"
	include "vendor/shaderc/libshaderc/include"

	dependency("libshaderc_util", "glslang")

subfile("../../../flint/flint.lua", "flint");

shared "plugin_glsl"
	path "glsl"

	threads()

	dependency("libshaderc", "flint")
