#include <shaderc/shaderc.hpp>

#include <iostream>

#include "logger.h"
// @todo Fix the include situation
//#include "C:\Users\timhu\Desktop\test\flint\third_party\logger\logger\include\logger.h"

std::vector<uint8_t> compile(const std::string& source_name, shaderc_shader_kind kind, const std::string& source, bool optimize = false) {
	shaderc::Compiler compiler;
	shaderc::CompileOptions options;

	options.AddMacroDefinition("MY_DEFINE", "1");
	if (optimize) options.SetOptimizationLevel(shaderc_optimization_level_size);

	shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, kind, source_name.c_str(), options);

	if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
		std::cerr << "ERROR\n";
		std::cerr << module.GetErrorMessage();
		throw std::runtime_error("Failed to build shader");
	}

	// This is kinda jenky, but otherwise the module will free the data
	// @todo This will fail if we have different endianess
	// Should use memstreams
	std::vector<uint8_t> result((module.end() - module.begin())*4);
	memcpy(result.data(), module.begin(), result.size());

	return result;
}

#include "flint.h"

#if _WIN32
	#define FLINT_PLUGIN __declspec(dllexport) __stdcall
#else
	#define FLINT_PLUGIN
#endif

extern "C" void FLINT_PLUGIN init(Flint& flint) {
	static bool a = false;
	if (!a) {
		logger::add(logger::std_logger);
		a = true;
	}

	sol::table helper = flint.test_get_lua().create_named_table("shader");
	helper.set_function("compiler", [] (std::vector<uint8_t> in) {
			std::string source(reinterpret_cast<const char*>(in.data()), in.size());

			std::vector<uint8_t> result = compile("shader_src", shaderc_glsl_infer_from_source, source, false);

			LOG_D("SIZE: {}\n", result.size());

			return result;
	});
}
