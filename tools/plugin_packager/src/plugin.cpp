#include "flint.h"

#include "packager/archive.h"

extern "C" void init() {
	// @todo Instead of passing in the name it should automatically be taken from the type
	flint::register_class<Archive>("archive");

	// @todo This is here temporarily until we replace langpack
	sol::table helper = flint::test_get_lua().create_named_table("helper");
	helper.set_function("new_byte_vector", []{
			return std::vector<uint8_t>();
	});

}
