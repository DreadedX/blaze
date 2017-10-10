#include "asset_list.h"
#include "engine.h"

#define LOGURU_IMPLEMENTATION 1
#include <loguru.hpp>

using namespace blaze;

void handle_chat_message(std::shared_ptr<ChatMessage> event) {
	std::cout << "<Dreaded_X> " << event->get_text() << '\n';;
}

int main(int argc, char** argv) {

	// @todo Make this part of the engine
	// Logging test
	{
		// This whill be 
		loguru::init(argc, argv);
		loguru::add_file("logs/everything.log", loguru::Append, loguru::Verbosity_MAX);
		loguru::add_file("logs/latest_readable.log", loguru::Truncate, loguru::Verbosity_INFO);

		// loguru::g_stderr_verbosity = 1;

		LOG_SCOPE_F(INFO, "Will indent all log messages within this scope.");
		LOG_F(INFO, "I'm hungry for some %.3f!", 3.14159);
		LOG_F(2, "Will only show if verbosity is 2 or higher");
		LOG_F(ERROR, "This is an error");
		// VLOG_F(get_log_level(), "Use vlog for dynamic log level (integer in the range 0-9, inclusive)");
		// LOG_IF_F(ERROR, badness, "Will only show if badness happens");
		// auto fp = fopen(filename, "r");
		// CHECK_F(fp != nullptr, "Failed to open file '%s'", filename);
		// CHECK_GT_F(length, 0); // Will print the value of `length` on failure.
		// CHECK_EQ_F(a, b, "You can also supply a custom message, like to print something: %d", a + b);
	}

	// Initialze engine
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
		flame::MetaAsset lua_asset("LuaTest", "assets/test.lua", 10, flame::MetaAsset::Workflow());
		flame::asset_list::add(lua_asset);
	}

	// asset_manager
	auto script = asset_manager::new_asset<LuaScript>("LuaTest");
	{
		auto nl = asset_manager::new_asset<LanguagePack>("Dutch");
		auto en = asset_manager::new_asset<LanguagePack>("English");

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
			
		std::cout << nl->get("PROFIT", {std::to_string(47)}) << '\n';
		std::cout << nl->get("NAMEAGE", {"Tim", std::to_string(19)}) << '\n';
		std::cout << nl->get("TEST") << '\n';
		std::cout << en->get("PROFIT", {std::to_string(47)}) << '\n';
		std::cout << en->get("NAMEAGE", {"Tim", std::to_string(19)}) << '\n';
		std::cout << en->get("TEST") << '\n';

	}

	// Event bus test
	{
		// Lua also registers a handler
		auto sub = event_bus::Subscription<ChatMessage>(std::ref(handle_chat_message));

		event_bus::send(std::make_shared<ChatMessage>("Hello world!"));
		event_bus::send(std::make_shared<ChatMessage>("This is a test"));
	}
}
