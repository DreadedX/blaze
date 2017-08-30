#include <fstream>
#include <iostream>

#include "archive.h"
#include "asset.h"
#include "binary_helper.h"
#include "async_data.h"

#include "rsa.h"
#include "osrng.h"
#include "integer.h"

blaze::flame::Asset::TaskData zlib_compress(blaze::flame::Asset::TaskData task_data) {
	std::cout << "This could be zlib compression\n";
	task_data.first = std::make_unique<uint8_t[]>(10);
	task_data.second = 10;
	for (int i = 0; i < 10; ++i) {
		task_data.first[i] = 9-i;
	}
	return task_data;
}

// Just to make everything compile
int main() {
	std::shared_ptr<blaze::flame::ASyncFStream> archive_file = std::make_shared<blaze::flame::ASyncFStream>("test.flm", std::ios::in | std::ios::out | std::ios::trunc);
	blaze::flame::Archive archive(archive_file, "Dreaded_X", "This is an archive just for testing the system");
	archive.initialize();

	// Now we can share one file stream and ensure that only one thread can read from it
	std::shared_ptr<blaze::flame::ASyncFStream> test_file = std::make_shared<blaze::flame::ASyncFStream>("test.txt", std::ios::in);
	blaze::flame::Asset test_asset("TestAsset", test_file, 1);
	blaze::flame::Asset test_asset2("TestAsset2", test_file, 1);

	test_asset.add_load_task(zlib_compress);

	auto data = test_asset.get_data();
	auto data2 = test_asset2.get_data();
	while (!data->is_loaded()) {
		std::cout << "Waiting for data to load!\n";
	}
	for (uint32_t i = 0; i < data->get_size(); ++i) {
		auto dat = (*data)[i];
		std::cout << dat;
	}
	for (uint32_t i = 0; i < data2->get_size(); ++i) {
		auto dat = (*data2)[i];
		std::cout << dat;
	}

	archive << test_asset;
	archive << test_asset2;

	archive.finialize();
	blaze::flame::Asset::wait_for_workers();
}
