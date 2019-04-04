#include "logger.h"

#include "engine.h"
#include "asset_manager.h"
#include "events.h"
#include "platform/platform.h"

#include "enviroment.h"

#include "game_asset/script.h"

// @todo THis is more for testing
#include "game_asset/language.h"
#include "events.h"

sol::state lua_state;
std::vector<std::shared_ptr<blaze::Script>> scripts;

std::unique_ptr<blaze::platform::Platform> blaze::current_platform;

sol::object lua_loader(std::string module_name) {
	// @todo This will block, but there is not really a way around it, unless we maybe make an indirect layer
	try {
		auto data = blaze::archive_manager::load_data(module_name);

		// Wait for the asset to load
		data.wait();

		return lua_state.load(data.get_as<std::string>());
	} catch (std::exception &e) {
		return sol::make_object(lua_state, "\n\tno asset '" + module_name + '\'');
	}
}

template <typename T>
void bind_event_subscription(sol::table& blaze, std::string name) {
	blaze.new_usertype<blaze::event_bus::Subscription<T>>(name,
			sol::constructors<
			blaze::event_bus::Subscription<T>(std::function<void(std::shared_ptr<T>)>)
		>(),
		"unsubscribe", &blaze::event_bus::Subscription<T>::unsubscribe
	);
}

namespace BLAZE_NAMESPACE {
	void init() {
		lua_state.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::table);

		// Add custom loader that allows loading from archives
		// @todo We should make sure these loaders are earlier
		sol::table searchers = lua_state["package"]["searchers"];
		searchers.add(&lua_loader);

		// Override print function
		// @todo Improve this
		// @todo Instead of using the macro we should call the function ourselfs so we can include file and line data
		lua_state.set_function("print", [](sol::variadic_args va) {
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

		sol::table blaze = lua_state.create_table("blaze");

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

	void load_archive(std::string archive_name) {
		std::string filename = current_platform->get_base_path() + archive_name + ".flm";

		try {
			flame::Archive archive(filename);

			archive_manager::add(archive);

			try {
				auto script = asset_manager::new_asset<Script>("/resources/" + archive.get_name() + "/Script");
				scripts.push_back(std::move(script));
			} catch (std::exception& e) {
				// @todo We should have a custom exception for this as we now assume an exception means not found
				LOG_D("Archive '{}' does not have a script.\n", archive.get_name());
			}
		} catch (std::exception& e) {
			event_bus::send(std::make_shared<Error>("Failed to load archive '" + archive_name + "': " + e.what(), __FILE__, __LINE__));
		}
	}

	void update() {
		for (auto& script : scripts) {
			script->update();
		}
	}

	void done() {
		scripts.clear();
		// @todo Make sure we clean everything up properly
	}

	sol::state& get_lua_state() {
		return lua_state;
	}

	std::unique_ptr<platform::Platform>& get_platform() {
		return current_platform;
	}
}
