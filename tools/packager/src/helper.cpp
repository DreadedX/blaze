#include "helper.h"
#include "binary_helper.h"

#include "external_task.h"

// Test
#include "trusted_key.h"

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
	return std::make_shared<FLAME_NAMESPACE::FileHandler>(path, std::ios::in | std::ios::out);
}

std::shared_ptr<FLAME_NAMESPACE::FileHandler> open_new_file(std::string path) {
	return std::make_shared<FLAME_NAMESPACE::FileHandler>(path, std::ios::in | std::ios::out | std::ios::trunc);
}

void bind(sol::state& lua) {
	lua.set_function("open_file", &open_file);
	lua.set_function("open_new_file", &open_new_file);
	lua.set_function("load_private_key", &load_private_key);
	lua.set_function("get_trusted_key", []{ return trusted_key; });

	lua.set_function("get_external_task", &get_external_task);

	lua.set_function("debug_content", [](FLAME_NAMESPACE::AssetData& data){
		std::cout << "Size: " << data.get_size() << '\n';
		for (uint32_t i = 0; i < data.get_size(); ++i) {
			auto dat = data[i];
			// std::cout << " 0x" << std::hex << (uint32_t)dat;
			std::cout << dat;
		}
	});
}
