#include "engine.h"
#include "events.h"

#include "bind_flame.h"

sol::state lua;

namespace BLAZE_NAMESPACE {

	namespace asset_manager {
		std::list<std::shared_ptr<GameAsset>> _private::loading_assets;

		void load_assets() {
			_private::loading_assets.remove_if([](std::shared_ptr<GameAsset> asset){
				bool loaded = asset->is_loaded();
				if (loaded) {
					asset->post_load();
				}
				return loaded;
			});
		}

		size_t loading_count() {
			return _private::loading_assets.size();
		}
	}

	template <typename T>
	void bind_event_subscription(sol::table& blaze, std::string name) {
		blaze.new_usertype<event_bus::Subscription<T>>(name,
			sol::constructors<
				event_bus::Subscription<T>(std::function<void(std::shared_ptr<T>)>)
			>(),
			"unsubscribe", &event_bus::Subscription<T>::unsubscribe
		);
	}

	// @todo This needs to be moved to a lua bind module, rename lua-flame to bind_lua and put all lua binding there
	void bind(sol::state& lua) {

		sol::table blaze = lua.create_table("blaze");

		bind_event_subscription<ChatMessage>(blaze, "ChatSubscription");
		blaze.new_usertype<ChatMessage>("ChatMessage", "get_text", &ChatMessage::get_text);
	}

	void initialize(std::initializer_list<std::string> archives) {
		lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string);
		flame::lua::bind(lua);
		bind(lua);

		for (auto& archive_name : archives) {
			// @note If we fail to open an archive we will tell the user but continue running as it might not be fatal
			try {
				auto fh = std::make_shared<flame::FileHandler>(archive_name, std::ios::in);
				flame::Archive archive(fh);

				flame::asset_list::add(archive);

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

	sol::state& get_lua_state() {
		return lua;
	}
}
