#include <fstream>
#include <iostream>

#include "archive.h"
#include "asset.h"
#include "binary_helper.h"
#include "async_data.h"

#include "compress.h"

#include "rsa.h"
#include "osrng.h"
#include "integer.h"

// Test
#include "trusted_key.h"
#include "asset_list.h"

// Just to make everything compile
int main() {
	{
		std::shared_ptr<blaze::flame::ASyncFStream> archive_file = std::make_shared<blaze::flame::ASyncFStream>("test.flm", std::ios::in | std::ios::out | std::ios::trunc);
		blaze::flame::Archive archive(archive_file, "Dreaded_X", "This is an archive just for testing the system");
		archive.initialize();

		// Now we can share one file stream and ensure that only one thread can read from it
		std::shared_ptr<blaze::flame::ASyncFStream> test_file = std::make_shared<blaze::flame::ASyncFStream>("assets/test.txt", std::ios::in);
		blaze::flame::Asset test_asset("TestAsset", test_file, 1);
		blaze::flame::Asset test_asset2("TestAsset2", test_file, 1);
		blaze::flame::Asset test_asset2new("TestAsset2", test_file, 2);

		test_asset.add_load_task(blaze::flame::zlib::compress);

		auto data = test_asset.get_data();
		auto data2 = test_asset2.get_data();
		while (!data.is_loaded()) {
			std::cout << "Waiting for data to load!\n";
			if (data.get_state() == blaze::flame::State::FAILED) {
				std::cerr << "Failed to load asset\n";
				break;
			}
		}
		for (uint32_t i = 0; i < data.get_size(); ++i) {
			auto dat = data[i];
			std::cout << dat;
		}
		for (uint32_t i = 0; i < data2.get_size(); ++i) {
			auto dat = data2[i];
			std::cout << dat;
		}

		archive << test_asset << test_asset << test_asset2new << test_asset2;

		std::fstream priv_key_file("priv.key", std::ios::in);
		std::shared_ptr<uint8_t[]> priv_key = std::make_unique<uint8_t[]>(1217);
		blaze::flame::binary::read(priv_key_file, priv_key.get(), 1217);
		archive.finialize(priv_key, 1217);
	}

	{
		std::shared_ptr<blaze::flame::ASyncFStream> archive_file = std::make_shared<blaze::flame::ASyncFStream>("test.flm", std::ios::in | std::ios::out);
		blaze::flame::Archive archive(archive_file);
		std::cout << "Author: " << archive.get_author() << '\n';
		std::cout << "Description: " << archive.get_description() << '\n';
		std::cout << "Official: " << (archive.is_trusted(trusted_key) ? "Yes" : "No") << '\n';

		blaze::flame::AssetList asset_list;
		asset_list << archive;

		asset_list.debug_list_assets();
		
		auto data = asset_list.find_asset("TestAsset");
		for (uint32_t i = 0; i < data.get_size(); ++i) {
			auto dat = data[i];
			std::cout << dat;
		}

		auto test_asset3 = asset_list.find_asset("TestAsset3");
		assert(test_asset3.get_state() == blaze::flame::State::FAILED);
	}
}
