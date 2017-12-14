#include "logger.h"

#include "helper.h"
#include "binary_helper.h"

#include "external_task.h"

// Test
#include "trusted_key.h"

#include <iostream>

crypto::RSA load_private_key(std::string path) {
	return crypto::load(path);
}

std::shared_ptr<FLAME_NAMESPACE::FileHandler> open_file(std::string path) {
	return std::make_shared<FLAME_NAMESPACE::FileHandler>(path, std::ios::in);
}

std::shared_ptr<FLAME_NAMESPACE::FileHandler> open_new_file(std::string path) {
	return std::make_shared<FLAME_NAMESPACE::FileHandler>(path, std::ios::in | std::ios::out | std::ios::trunc);
}

void bind(sol::state& lua) {
	sol::table helper = lua.create_named_table("helper");
	helper.set_function("new_byte_vector", []{
			return std::vector<uint8_t>();
	});
	helper.set_function("load_private_key", &load_private_key);
	helper.set_function("get_trusted_key", &get_trusted_key);

	helper.set_function("get_external_task", &get_external_task);

	helper.set_function("debug_content", [](FLAME_NAMESPACE::AssetData& data) {
		LOG_D("Size: {}\n", data.get_size());
		LOG_D("Content: \n", data.get_size());
		for (uint32_t i = 0; i < data.get_size(); ++i) {
			auto dat = data[i];
			LOG_D("{}", dat);
		}
		LOG_D("\n");
	});
}
