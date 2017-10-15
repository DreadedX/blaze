#include "asset_list.h"
#include "engine.h"

#include "version.h"

using namespace blaze;

void handle_chat_message(std::shared_ptr<ChatMessage> event) {
	std::cout << "<Dreaded_X> " << event->get_text() << '\n';
}

void handle_missing_dependencies(std::shared_ptr<MissingDependencies> event) {
	std::cout << "Archive '" << event->get_name() << "' is missing the following dependencies:\n";
	for (auto dependency : event->get_missing()) {
		std::cout << dependency.first << ':' << dependency.second << '\n';
	}
}

void lang_test(std::shared_ptr<blaze::Language> lang) {
		std::cout << lang->get("tutorial.part1") << '\n';

		std::cout << lang->get("pickaxe.name") << '\n';
		std::cout << lang->get("pickaxe.description", {47, 100}) << '\n';

}

int main() {
	std::cout << "Version number: " << get_version_number() <<'\n';
	std::cout << "Version string: " << get_version_string() <<'\n';

	// Initialze engine
	// @todo Make it so we do not have to keep a reference around if we just want to register and forget
	auto asdf = event_bus::Subscription<MissingDependencies>(std::ref(handle_missing_dependencies));
	blaze::initialize({"archives/base.flm", "archives/test.flm"});

	// Flame tests
	{
		std::cout << "====ASSETS====\n";
		flame::asset_list::debug_list_meta_assets();
		std::cout << "==============\n";
	}

	// Override LuaTest in archive with version from disk
	{
		// @todo This should go into a lua script
		flame::MetaAsset lua_asset("Test", "assets/script/Test.lua", 10, flame::MetaAsset::Workflow());
		flame::asset_list::add(lua_asset);
	}

	// asset_manager
	auto script = asset_manager::new_asset<Script>("Test");
	auto en = asset_manager::new_asset<Language>("English");
	get_lua_state().set_function("get_lang", [en]{
		return en;
	});
	{
		auto nl = asset_manager::new_asset<Language>("Dutch");

		// Wait for all gameassets to be loaded and show progress
		auto total_count = asset_manager::loading_count();
		auto not_loaded_count = total_count;
		while (not_loaded_count  > 0) {
			std::cout << "Loaded assets: " << total_count-not_loaded_count << '/' << total_count << '\n';
			asset_manager::load_assets();

			// Example of a loading screen
			not_loaded_count = asset_manager::loading_count();
		}
		std::cout << "Loaded assets: " << total_count-not_loaded_count << '/' << total_count << '\n';

		// Simulate the core game loop
		for (int i = 0; i < 3; ++i) {
			script->update();
		}
			
		lang_test(en);
		lang_test(nl);
	}

	// Event bus test
	{
		// Lua also registers a handler
		auto sub = event_bus::Subscription<ChatMessage>(std::ref(handle_chat_message));

		event_bus::send(std::make_shared<ChatMessage>("Hello world!"));
		event_bus::send(std::make_shared<ChatMessage>("This is a test"));
	}
}
