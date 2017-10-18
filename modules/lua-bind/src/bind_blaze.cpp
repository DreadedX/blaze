#include "bind_blaze.h"

#include "events.h"
#include "game_asset.h"

namespace BLAZE_NAMESPACE::lua {
	template <typename T>
	void bind_event_subscription(sol::table& blaze, std::string name) {
		blaze.new_usertype<event_bus::Subscription<T>>(name,
			sol::constructors<
				event_bus::Subscription<T>(std::function<void(std::shared_ptr<T>)>)
			>(),
			"unsubscribe", &event_bus::Subscription<T>::unsubscribe
		);
	}

	void bind(sol::state& lua) {

		sol::table blaze = lua.create_table("blaze");

		bind_event_subscription<ChatMessage>(blaze, "ChatSubscription");
		blaze.new_usertype<ChatMessage>("ChatMessage",
			sol::base_classes, sol::bases<Event>(),
			"text", sol::property(&ChatMessage::get_text)
		);

		blaze.new_usertype<Language>("Language",
			sol::base_classes, sol::bases<GameAsset>(),
			"get", &Language::get<sol::variadic_args>
		);
	}

}
