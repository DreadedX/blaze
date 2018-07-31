#include "logger.h"

#include "helper.h"
#include "binary_helper.h"

// Test
#include "trusted_key.h"

#include <iostream>
// @todo This will be really janky on other platforms
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem::v1;

crypto::RSA load_private_key(std::string path) {
	return crypto::load(path);
}

std::shared_ptr<FLAME_NAMESPACE::FileHandler> open_file(std::string path) {
	return std::make_shared<FLAME_NAMESPACE::FileHandler>(path, std::ios::in | std::ios::binary);
}

std::shared_ptr<FLAME_NAMESPACE::FileHandler> open_new_file(std::string path) {
	return std::make_shared<FLAME_NAMESPACE::FileHandler>(path, std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
}

void bind(sol::state& lua) {
	sol::table helper = lua.create_named_table("helper");
	helper.set_function("new_byte_vector", []{
			return std::vector<uint8_t>();
	});
	helper.set_function("load_private_key", &load_private_key);
	helper.set_function("get_trusted_key", &get_trusted_key);

	helper.set_function("debug_content", [](FLAME_NAMESPACE::AssetData& data) {
		LOG_D("Size: {}\n", data.get_size());
		LOG_D("Content: \n", data.get_size());
		for (uint32_t i = 0; i < data.get_size(); ++i) {
			auto dat = data[i];
			LOG_D("{}", dat);
		}
		LOG_D("\n");
	});

	helper.set_function("list_files", [&lua](std::string path) {
		fs::path base(path);
		sol::table files = lua.create_table();

		for (const auto& entry : fs::directory_iterator(base)) {
			if (!fs::is_directory(entry)) {
				if (fs::path(entry).extension() == ".cpp" || fs::path(entry).extension() == ".cc" || fs::path(entry).extension() == ".c") {
					files.add((fs::current_path() / entry).string());
				}
			}
		}

		return files;
	});

	helper.set_function("list_files_recursive", [&lua](std::string path) {
		fs::path base(path);
		sol::table files = lua.create_table();

		for (const auto& entry : fs::recursive_directory_iterator(base)) {
			if (!fs::is_directory(entry)) {
				if (fs::path(entry).extension() == ".cpp" || fs::path(entry).extension() == ".cc" || fs::path(entry).extension() == ".c") {
					files.add((fs::current_path() / entry).string());
				}
			}
		}

		return files;
	});

	helper.set_function("native_path", [](std::string path) {
		return (fs::current_path() / path).string();
	});

	helper.set_function("change_extension", [](std::string path, std::string extension) {
		return fs::path(path).stem().string() + extension;
	});

	helper.set_function("get_extension", [](std::string path) {
		return fs::path(path).extension().string();
	});

	helper.set_function("create_directory", [](std::string path) {
		return fs::create_directory(fs::path(path));
	});

	helper.set_function("copy", [](std::string source, std::string dest) {
		return fs::copy(fs::path(source), fs::path(dest), fs::copy_options::overwrite_existing);
	});
}
