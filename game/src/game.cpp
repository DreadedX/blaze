#include "logger.h"

#include "asset_list.h"
#include "engine.h"
#include "asset_manager.h"

#include "enviroment.h"

using namespace blaze;

void handle_chat_message(std::shared_ptr<ChatMessage> event) {
	log(Level::debug, "<Dreaded_X> {}\n", event->get_text());
}

void handle_missing_dependencies(std::shared_ptr<MissingDependencies> event) {
	log(Level::error, "Archive '{}', is missing the following dependencies:\n");
	for (auto dependency : event->get_missing()) {
		log(Level::error, "{}:{}\n", dependency.first, dependency.second);
	}
}

void handle_error(std::shared_ptr<Error> event) {
	log(Level::error, "{}\n=>\t{}\n", event->get_context(), event->get_error());
}

void lang_test(std::shared_ptr<blaze::Language> lang) {
		log(Level::debug, "{}\n", lang->get("tutorial.part1"));

		log(Level::debug, "{}\n", lang->get("pickaxe.name"));
		log(Level::debug, "{}\n", lang->get("pickaxe.description", {47, 100}));

}

void game() {
	// Setup event handlers
	event_bus::subscribe<MissingDependencies>(std::ref(handle_missing_dependencies));
	event_bus::subscribe<Error>(std::ref(handle_error));

	// Override assets so we don't have to repackage everytime
	if constexpr (blaze::enviroment::os == blaze::enviroment::OS::Linux) {
		// @todo This should go into a lua script
		flame::MetaAsset lua_asset("base/Script", "../assets/base/script/Script.lua", 10);
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
			asset_manager::load_assets();

			// Example of a loading screen
			not_loaded_count = asset_manager::loading_count();
		}

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
		log(Level::debug, "{}\n", "====ASSETS====");
		blaze::asset_list::debug_list_meta_assets();
		log(Level::debug, "{}\n", "==============");
	}

	// Event bus test
	{
		// Lua also registers a handler
		auto sub = event_bus::Subscription<ChatMessage>(std::ref(handle_chat_message));

		event_bus::send(std::make_shared<ChatMessage>("Hello world!"));
		event_bus::send(std::make_shared<ChatMessage>("This is a test"));
	}
}
