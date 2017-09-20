#include "bind-flame.h"
#include "helper.h"

// Just to make everything compile
int main() {

	using namespace blaze::flame;

	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::io, sol::lib::string);

	lua::bind(lua);
	bind(lua);

	lua.script_file("packager.lua");

	return 0;

	#if 0
		Archive archive(open_new_file("test.flm"), "test", "Dreaded_X", "This is an archive just for testing the system", 1);
		// @todo We need to make a second archive to test this stuff
		archive.add_dependency("test", 1);
		archive.initialize();

		// Now we can share one file stream and ensure that only one thread can read from it
		auto test_file = open_file("assets/test.txt");
		Asset test_asset("TestAsset", test_file, 1);
		Asset test_asset2("TestAsset2", test_file, 1);
		Asset test_asset2new("TestAsset2", test_file, 2);

		Asset::Workflow workflow;
		workflow.inner.push_back(zlib::compress);
		test_asset.set_workflow(workflow);

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

		auto priv_key = load_private_key("keys/test.priv");

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
	#endif
}

