#include "flint.h"

#include "packager/archive.h"

#if _WIN32
	#define FLINT_PLUGIN __declspec(dllexport) __stdcall
#else
	#define FLINT_PLUGIN
#endif

extern "C" void FLINT_PLUGIN init(Flint& flint) {
	// @todo Instead of passing in the name it should automatically be taken from the type
	flint.register_class<Archive>("archive");

	// @todo This is here temporarily until we replace langpack
	sol::table helper = flint.test_get_lua().create_named_table("helper");
	helper.set_function("new_byte_vector", []{
			return std::vector<uint8_t>();
	});

}
