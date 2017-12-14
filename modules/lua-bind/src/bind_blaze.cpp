#include "bind_blaze.h"

#include "events.h"
#include "game_asset.h"
#include "engine.h"

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

	sol::object loader(std::string module_name) {
		// @todo This will block, but there is not really a way around it, unless we maybe make an indirect layer
		try {
			auto data = asset_list::find_asset(module_name);
			return get_lua_state().load(data.as<std::string>());
		} catch (std::exception &e) {
			return sol::make_object(get_lua_state(), "\n\tno asset '" + module_name + '\'');
		}
	}

	// @todo It would be nice if we could somehow extract the switch statements
	void bind(sol::state& lua) {
		// Add custom loader that allows loading from archives
		sol::table searchers = lua["package"]["searchers"];
		searchers.add(&loader);

		// Override print function
		// @todo Improve this
		lua.set_function("print", [](sol::variadic_args va) {
			for (auto v : va) {
				switch (v.get_type()) {
					case sol::type::nil:
						LOG_M("nil\t");
						break;

					case sol::type::string:
						LOG_M( "{}\t", v.get<std::string>());
						break;

					case sol::type::number:
						LOG_M( "{}\t", v.get<double>());
						break;

					case sol::type::boolean:
						LOG_M( "{}\t", v.get<bool>());
						break;

					default:
						LOG_M( "<<NOT IMPLEMENTED>>\t");
						break;
				}
			}
			LOG_M( "\n");
		});

		sol::table blaze = lua.create_table("blaze");

		bind_event_subscription<ChatMessage>(blaze, "ChatSubscription");
		blaze.new_usertype<ChatMessage>("ChatMessage",
			sol::base_classes, sol::bases<Event>(),
			"text", sol::property(&ChatMessage::get_text)
		);

		blaze.new_usertype<Language>("Language",
			sol::base_classes, sol::bases<GameAsset>(),
			// We need some extra code to convert the variadic args to a fmt ArgList
			"get", [](Language& base, std::string name, sol::variadic_args va) {
				// @todo Make sure we the amount of arguments does not exceed the max
				typedef fmt::internal::ArgArray<fmt::ArgList::MAX_PACKED_ARGS-1> ArgArray;
				typename ArgArray::Type array;
				uint64_t ArgArrayType = 0;

				int i = 0;
				for (auto v : va) {
					switch (v.get_type()) {
						case sol::type::nil:
							array[i] = ArgArray::template make<fmt::BasicFormatter<char>>("nil");
							ArgArrayType = ArgArrayType | (fmt::internal::make_type(" ") << (i*4));
							break;

						case sol::type::string:
							array[i] = ArgArray::template make<fmt::BasicFormatter<char>>(v.get<const char*>());
							ArgArrayType = ArgArrayType | (fmt::internal::make_type(" ") << (i*4));
							break;

						case sol::type::number:
							array[i] = ArgArray::template make<fmt::BasicFormatter<char>>(v.get<double>());
							ArgArrayType = ArgArrayType | (fmt::internal::make_type(1.0) << (i*4));
							break;

						case sol::type::boolean:
							array[i] = ArgArray::template make<fmt::BasicFormatter<char>>(v.get<bool>());
							ArgArrayType = ArgArrayType | (fmt::internal::make_type(true) << (i*4));
							break;

						default:
							array[i] = ArgArray::template make<fmt::BasicFormatter<char>>("<<NOT IMPLEMENTED>>");
							ArgArrayType = ArgArrayType | (fmt::internal::make_type(" ") << (i*4));
							break;
					}
					++i;
				}

				fmt::ArgList args(ArgArrayType, array);
				return base.get(name, args);
			}
		);

		blaze.set_function("load_archive", &load_archive);
	}
}
