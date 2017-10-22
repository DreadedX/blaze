#include "helper.h"
#include "binary_helper.h"

#include "external_task.h"

// Test
#include "trusted_key.h"

#include <iostream>

std::array<uint8_t, 1217> load_private_key(std::string path) {
	std::fstream priv_key_file(path, std::ios::in);
	std::array<uint8_t, 1217> priv_key;
	if (priv_key_file.is_open()) {
		FLAME_NAMESPACE::binary::read(priv_key_file, priv_key);
	} else {
		std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "Failed to open key file\n";
	}
	return priv_key;
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
	helper.set_function("open_file", &open_file);
	helper.set_function("open_new_file", &open_new_file);
	helper.set_function("load_private_key", &load_private_key);
	helper.set_function("get_trusted_key", &get_trusted_key);

	helper.set_function("get_external_task", &get_external_task);

	helper.set_function("debug_content", [](FLAME_NAMESPACE::AssetData& data){
		std::cout << "Size: " << data.get_size() << '\n';
		std::cout << "Content: " << '\n';
		for (uint32_t i = 0; i < data.get_size(); ++i) {
			auto dat = data[i];
			std::cout << dat;
		}
		std::cout << '\n';
	});
}
