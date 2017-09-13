#include <fstream>
#include <iostream>

#include "archive.h"
#include "asset.h"
#include "binary_helper.h"
#include "async_data.h"

#include "compress.h"

// CryptoPP
#include "rsa.h"
#include "osrng.h"
#include "integer.h"

// Sol2
#include "sol.hpp"

// Test
#include "trusted_key.h"
#include "asset_list.h"

auto load_private_key(std::string path) {
	std::fstream priv_key_file(path, std::ios::in);
	std::array<uint8_t, 1217> priv_key;
	blaze::flame::binary::read(priv_key_file, priv_key);
	return priv_key;
}

inline auto open_file(std::string path) {
	return std::make_shared<blaze::flame::ASyncFStream>(path, std::ios::in | std::ios::out);
}

inline auto open_new_file(std::string path) {
	return std::make_shared<blaze::flame::ASyncFStream>(path, std::ios::in | std::ios::out | std::ios::trunc);
}

void lua_test() {
	using namespace blaze::flame;
	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::io, sol::lib::string);

	lua.new_usertype<Asset> ("Asset",
			sol::constructors<
				Asset(std::string, std::shared_ptr<ASyncFStream>, uint16_t),
				Asset(std::string, std::shared_ptr<ASyncFStream>, uint16_t, uint32_t, uint32_t)
				>(),
				"get_name", &Asset::get_name,
				"get_version", &Asset::get_version
			);
	lua.new_usertype<Archive> ("Archive",
			sol::constructors<
				Archive(std::shared_ptr<ASyncFStream> afs, std::string, std::string, std::string, uint16_t),
				Archive(std::shared_ptr<ASyncFStream> afs)
			>(),
			"add_dependency", &Archive::add_dependency,
			"initialize", &Archive::initialize,
			"finalize", &Archive::finialize,
			"add", &Archive::add
			);
	lua.new_usertype<ASyncFStream> ("ASyncFStream",
			"is_open", &ASyncFStream::is_open
			);

	lua.set_function("open_file", &open_file);
	lua.set_function("open_new_file", &open_new_file);
	lua.set_function("load_private_key", &load_private_key);

	lua.script_file("packager.lua");
}

// Just to make everything compile
int main() {

	using namespace blaze::flame;

	lua_test();

	return 0;

	{
		Archive archive(open_new_file("test.flm"), "test", "Dreaded_X", "This is an archive just for testing the system", 1);
		// @todo We need to make a second archive to test this stuff
		archive.add_dependency("test", 1);
		archive.initialize();

		// Now we can share one file stream and ensure that only one thread can read from it
		auto test_file = open_file("assets/test.txt");
		Asset test_asset("TestAsset", test_file, 1);
		Asset test_asset2("TestAsset2", test_file, 1);
		Asset test_asset2new("TestAsset2", test_file, 2);

		test_asset.add_load_task(zlib::compress);

		auto data = test_asset.get_data();
		auto data2 = test_asset2.get_data();
		while (!data.is_loaded()) {
			std::cout << "Waiting for data to load!\n";
			if (data.get_state() == State::FAILED) {
				std::cerr << "Failed to load asset\n";
				break;
			}
		}
		for (uint32_t i = 0; i < data.get_size(); ++i) {
			auto dat = data[i];
			std::cout << dat;
		}
		std::cout << '\n';
		for (uint32_t i = 0; i < data2.get_size(); ++i) {
			auto dat = data2[i];
			std::cout << dat;
		}
		std::cout << '\n';

		archive.add(test_asset);
		archive.add(test_asset);
		archive.add(test_asset2new);
		archive.add(test_asset2);

		auto priv_key = load_private_key("priv.key");

		archive.finialize(priv_key);
	}

	{
		Archive archive(open_file("test.flm"));
		std::cout << "Name: " << archive.get_name() << '\n';
		std::cout << "Author: " << archive.get_author() << '\n';
		std::cout << "Description: " << archive.get_description() << '\n';
		std::cout << "Version: " << archive.get_version() << '\n';
		std::cout << "Official: " << (archive.is_trusted(trusted_key) ? "Yes" : "No") << '\n';

		std::cout << "Dependencies: \n";
		for (auto& dependency : archive.get_dependencies()) {
			std::cout << '\t' << dependency.first << ' ' << dependency.second << '\n';
		}

		AssetList asset_list;
		// Add archive to a list of archives to load
		asset_list.add(archive);
		// This function is needed to prevent cyclic dependencies
		asset_list.load_archives();

		Asset test_asset("TestAssetNOARCHIVE", open_file("assets/test.txt"), 1);
		asset_list.add(test_asset);

		asset_list.debug_list_assets();
		
		auto data = asset_list.find_asset("TestAsset");
		for (uint32_t i = 0; i < data.get_size(); ++i) {
			auto dat = data[i];
			std::cout << dat;
		}

		auto test_asset3 = asset_list.find_asset("TestAsset3");
		assert(test_asset3.get_state() == State::FAILED);
	}
}
