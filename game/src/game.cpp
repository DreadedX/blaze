#include "logger.h"

#include "asset_list.h"
#include "engine.h"
#include "asset_manager.h"

#include "enviroment.h"

#include "game_asset_types.h"

using namespace blaze;

void handle_chat_message(std::shared_ptr<ChatMessage> event) {
	LOG_M("<Dreaded_X> {}\n", event->get_text());
}

void handle_missing_dependencies(std::shared_ptr<MissingDependencies> event) {
	LOG_E("Archive '{}', is missing the following dependencies:\n", event->get_name());
	for (auto dependency : event->get_missing()) {
		uint16_t version_min = std::get<1>(dependency);
		uint16_t version_max = std::get<2>(dependency);

		if (version_min != 0 && version_min != version_max) {
			LOG_E("{} <= ", version_min);
		}

		LOG_E("{}", std::get<0>(dependency));

		if (version_max != 0) {
			if (version_min != version_max) {
				LOG_E(" <= {}", version_max);
			} else {
				LOG_E(" = {}", std::get<2>(dependency));
			}
		}
		LOG_E("\n");
	}
}

void handle_error(std::shared_ptr<Error> event) {
	LOG_E("{}\n=>\t{}\n", event->get_context(), event->get_error());
}

void lang_test(std::shared_ptr<blaze::Language> lang) {
		LOG_M("{}\n", lang->get("tutorials.basic.1"));

		LOG_M("{}\n", lang->get("items.pickaxe.name"));
		LOG_M("{}\n", lang->get("items.pickaxe.description", 47, 100));

}

#define FORCE_UNDEFINED_SYMBOL(x) void* __ ## x ## _fp =(void*)&x;
void game() {
	#ifdef __ANDROID__
		// @todo This is needed because otherwise it is not included
		// Add export command to flint where we can add things that need to be exported
		FORCE_UNDEFINED_SYMBOL(JNI_OnLoad);
	#endif

	// Setup event handlers
	event_bus::subscribe<MissingDependencies>(std::ref(handle_missing_dependencies));
	event_bus::subscribe<Error>(std::ref(handle_error));

	// Override assets so we don't have to repackage everytime
	// if constexpr (blaze::enviroment::os == blaze::enviroment::OS::Linux && blaze::enviroment::debug) {
	// 	// @todo We can propably reuse the packager script if we override some of the functions that are used to create archives
	// 	flame::MetaAsset lua_asset("base/Script", "../assets/base/script/Script.lua", 10);
	// 	blaze::asset_list::add(lua_asset);
	// }

	// Load base archive
	blaze::load_archive("base");
	while (asset_manager::loading_count()  > 0) {
		asset_manager::load_assets();
	}

	auto en = asset_manager::new_asset<Language>("base/language/English");
	get_lua_state().set_function("get_lang", [en]{
		return en;
	});

	{
		auto nl = asset_manager::new_asset<Language>("base/language/Dutch");

		// Wait for all gameassets to be loaded and show progress
		while (asset_manager::loading_count()  > 0) {
			asset_manager::load_assets();
		}

		// Simulate the core game loop
		for (int i = 0; i < 3; ++i) {
			update();
		}
			
		lang_test(en);
		lang_test(nl);
	}

	// Flame tests
	{
		LOG_D("{}\n", "====ASSETS====");
		blaze::asset_list::debug_list_meta_assets();
		LOG_D("{}\n", "==============");
	}

	// Event bus test
	{
		// Lua also registers a handler
		auto sub = event_bus::Subscription<ChatMessage>(std::ref(handle_chat_message));

		event_bus::send(std::make_shared<ChatMessage>("Hello world!"));
		event_bus::send(std::make_shared<ChatMessage>("This is a test"));
	}
}
