#include "asset_list.h"
#include "engine.h"

using namespace blaze;

void test() {
	flame::AssetList asset_list;

	std::vector<std::string> archives = {"archives/test.flm", "archives/base.flm"};

	for (auto archive_name : archives) {
		auto archive_file = std::make_shared<flame::FileHandler>(archive_name, std::ios::in);
		flame::Archive archive(archive_file);
		asset_list.add(archive);
	}

	asset_list.load_archives();
	flame::AssetData test = asset_list.find_asset("TestAsset");

	std::cout << "====ASSETS====\n";
	asset_list.debug_list_meta_assets();

	std::cout << "====TEST====\n";
	for (unsigned int i = 0; i < test.get_size(); ++i) {
		std::cout << test[i];
	}
	std::cout << '\n';

}

int main() {
	// test();

	blaze::initialize({"archives/test.flm", "archives/base.flm"});

	LuaScript script("LuaTest");

	script.run();
	script.run();
}
