#include "engine.h"
#include "events.h"

#include "bind_flame.h"

sol::state lua;
FLAME_NAMESPACE::AssetList asset_list;
BLAZE_NAMESPACE::EventBus event_bus;

namespace BLAZE_NAMESPACE {

	void bind(sol::state& lua) {
		lua.set_function("subscribe_chat_event", [](std::function<void(std::shared_ptr<blaze::ChatMessage>)> handler) {
			return get_event_bus().subscribe<ChatMessage>(handler);
		});
		lua.set_function("unsubscribe_chat_event", [](std::list<std::function<void(std::shared_ptr<blaze::Event>)>>::iterator it) {
			get_event_bus().unsubscribe<ChatMessage>(it);
		});

		lua.new_usertype<ChatMessage>("ChatMessage",
			"get_text", &ChatMessage::get_text
		);
	}

	void initialize(std::initializer_list<std::string> archives) {
		lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string);
		FLAME_NAMESPACE::lua::bind(lua);
		bind(lua);

		for (auto& archive_name : archives) {
			// @note If we fail to open an archive we will tell the user but continue running as it might not be fatal
			try {
				auto fh = std::make_shared<FLAME_NAMESPACE::FileHandler>(archive_name, std::ios::in);
				FLAME_NAMESPACE::Archive archive(fh);

				asset_list.add(archive);

				// This needs to be run when catching a special exception that returns a list of missing dependencies
				// // We do not add the archive if it is missing a dependecy
				// // @todo Post a event in a central message bus
				// std::cerr << __FILE__ << ':' << __LINE__ << " =>\n\t" << "Missing dependencies:\n";
				// for (auto& missing : missing_dependecies) {
				// 	std::cerr << "\t\t" << missing.first << ':' << missing.second << '\n';
				// }

			} catch (std::exception& e) {
				std::cerr << __FILE__ << ':' << __LINE__ << " =>\n\t" << "Failed to open '" << archive_name << "': " << e.what() << '\n';
				// @todo Post a event in a central message bus
			}
		}
	}

	FLAME_NAMESPACE::AssetList& get_asset_list() {
		return asset_list;
	}

	sol::state& get_lua_state() {
		return lua;
	}

	EventBus& get_event_bus() {
		return event_bus;
	}
}
