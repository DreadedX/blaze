#include "asset_list.h"
#include "engine.h"
#include "asset_manager.h"

#include "enviroment.h"

using namespace blaze;

void handle_chat_message(std::shared_ptr<ChatMessage> event) {
	std::cout << "<Dreaded_X> " << event->get_text() << '\n';
}

void handle_missing_dependencies(std::shared_ptr<MissingDependencies> event) {
	std::cerr << "Archive '" << event->get_name() << "' is missing the following dependencies:\n";
	for (auto dependency : event->get_missing()) {
		std::cerr << dependency.first << ':' << dependency.second << '\n';
	}
}

void handle_error(std::shared_ptr<Error> event) {
	std::cerr << event->get_context() << "\n=>\t " << event->get_error() << '\n';
}

void lang_test(std::shared_ptr<blaze::Language> lang) {
		std::cout << lang->get("tutorial.part1") << '\n';

		std::cout << lang->get("pickaxe.name") << '\n';
		std::cout << lang->get("pickaxe.description", {47, 100}) << '\n';

}

void game() {
	// Setup event handlers
	event_bus::subscribe<MissingDependencies>(std::ref(handle_missing_dependencies));
	event_bus::subscribe<Error>(std::ref(handle_error));

	// Override assets so we don't have to repackage everytime
	if constexpr (enviroment::os == enviroment::OS::Linux) {
		// @todo This should go into a lua script
		flame::MetaAsset lua_asset("base/Script", "assets/base/script/Script.lua", 10);
		blaze::asset_list::add(lua_asset);
	}

	// Load base archive
	blaze::load_archive("base");

	auto en = asset_manager::new_asset<Language>("base/language/English");
	get_lua_state().set_function("get_lang", [en]{
		return en;
	});

	{
		auto nl = asset_manager::new_asset<Language>("base/language/Dutch");

		// Wait for all gameassets to be loaded and show progress
		auto total_count = asset_manager::loading_count();
		auto not_loaded_count = total_count;
		while (not_loaded_count  > 0) {
			// std::cout << "Loaded assets: " << total_count-not_loaded_count << '/' << total_count << '\n';
			asset_manager::load_assets();

			// Example of a loading screen
			not_loaded_count = asset_manager::loading_count();
		}
		// std::cout << "Loaded assets: " << total_count-not_loaded_count << '/' << total_count << '\n';

		// Simulate the core game loop
		for (int i = 0; i < 3; ++i) {
			update();
			// script->update();
		}
			
		lang_test(en);
		lang_test(nl);
	}

	// Flame tests
	{
		std::cout << "====ASSETS====\n";
		blaze::asset_list::debug_list_meta_assets();
		std::cout << "==============\n";
	}

	// Event bus test
	{
		// Lua also registers a handler
		auto sub = event_bus::Subscription<ChatMessage>(std::ref(handle_chat_message));

		event_bus::send(std::make_shared<ChatMessage>("Hello world!"));
		event_bus::send(std::make_shared<ChatMessage>("This is a test"));
	}
}
