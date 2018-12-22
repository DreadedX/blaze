lib "spirv-headers"
	include "vendor/spirv-headers/include"

lib "spirv-tools1"
	src "*vendor/spirv-tools/source/comp"
	include "vendor/spirv-tools/include"
	include "vendor/spirv-tools"
	include "vendor/headers"

	dependency "spirv-headers"

lib "spirv-tools2"
	src "*vendor/spirv-tools/source/link"
	include "vendor/spirv-tools/include"
	include "vendor/spirv-tools"
	include "vendor/headers"

	dependency "spirv-headers"

lib "spirv-tools3"
	src "*vendor/spirv-tools/source/opt"
	include "vendor/spirv-tools/include"
	include "vendor/spirv-tools"
	include "vendor/headers"

	dependency "spirv-headers"

lib "spirv-tools4"
	src "*vendor/spirv-tools/source/reduce"
	include "vendor/spirv-tools/include"
	include "vendor/spirv-tools"
	include "vendor/headers"

	dependency "spirv-headers"

lib "spirv-tools5"
	src "*vendor/spirv-tools/source/util"
	include "vendor/spirv-tools/include"
	include "vendor/spirv-tools"
	include "vendor/headers"

	dependency "spirv-headers"

lib "spirv-tools6"
	src "*vendor/spirv-tools/source/val"
	include "vendor/spirv-tools/include"
	include "vendor/spirv-tools"
	include "vendor/headers"

	dependency "spirv-headers"

lib "spirv-tools"
	src "*vendor/spirv-tools/source"
	include "vendor/spirv-tools/include"
	include "vendor/spirv-tools"
	include "vendor/headers"

	dependency("spirv-tools1", "spirv-tools2", "spirv-tools3", "spirv-tools4", "spirv-tools5", "spirv-tools6")

	dependency "spirv-headers"

lib "libshaderc_util"
	src "vendor/shaderc/libshaderc_util/src/{compiler,file_finder,io,message,resources,shader_stage,spirv_tools_wrapper,version_profile}.cc"
	include "vendor/shaderc/libshaderc_util/include"

	include "vendor/glslang"

	dependency "spirv-tools"

lib "glslang"
	src "**vendor/glslang/glslang/GenericCodeGen"
	src "**vendor/glslang/glslang/MachineIndependent"
	src "**vendor/glslang/SPIRV"
	if config.platform.target == "windows" then
		src "vendor/glslang/glslang/OSDependent/Windows/ossource.cpp"
	else
		src "vendor/glslang/glslang/OSDependent/Unix/ossource.cpp"
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

	dependency("libshaderc", "flint_base")
