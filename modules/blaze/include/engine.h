#pragma once

#include "blaze.h"

#include "asset_list.h"
#include "events.h"

#include "binary_helper.h"

#include "sol/state.hpp"

#include <initializer_list>
#include <string>

// @todo Refactor the shit out of this, most of this is just the first iteration
namespace BLAZE_NAMESPACE {
	void initialize(std::initializer_list<std::string> archives);
	sol::state& get_lua_state();

	class GameAsset {
		public:
			GameAsset(std::string asset_name) : _data(flame::asset_list::find_asset(asset_name)) {}
			virtual ~GameAsset() {}

			// This function gets called after the data has been loaded from disk
			virtual void post_load() = 0;

			bool is_loaded() {
				return _data.is_loaded();
			}

		protected:
			flame::AssetData _data;
	};

	namespace asset_manager {
		namespace _private {
			extern std::list<std::shared_ptr<GameAsset>> loading_assets;
		}

		template <typename T, typename... Args>
		std::shared_ptr<T> new_asset(std::string asset_name, Args... args) {
			static_assert(std::is_base_of<GameAsset, T>(), "T must be derived from GameAsset");
			std::shared_ptr<T> game_asset = std::make_shared<T>(asset_name, args...);
			_private::loading_assets.push_back(game_asset);
			return game_asset;
		}

		void load_assets();
		size_t loading_count();
	}

	class LuaScript : public GameAsset {
		public:
			LuaScript(std::string asset_name) : GameAsset(asset_name), environment(get_lua_state(), sol::create, get_lua_state().globals()) {}

			~LuaScript() {
				// Should always be true as loading_assets will have a valid reference to this object until it is loaded
				if (_loaded) {
					environment["done"]();
				}
			}

			// Gets called after the data has actually loaded, so it will not block
			void post_load() override {
				get_lua_state().safe_script(reinterpret_cast<const char*>(_data.data()), environment);
				_loaded = true;

				environment["init"]();
			}

			void update() {
				if (_loaded) {
					environment["update"]();
				} else {
					std::cerr << "Asset not loaded!\n";
				}
			}

		private:

			sol::environment environment;

			bool _loaded = false;
	};

	class LanguagePack : public GameAsset {
		public:
			LanguagePack(std::string asset_name) : GameAsset(asset_name) {}

			void post_load() {
				auto size = _data.get_size();
				uint32_t current = 0;

				std::cout << "size: " << size << '\n';

				// @todo This is a terrible way to do it
				while (current < size) {
					auto next = _data[current];
					std::string name = "";
					while (next != 0x0) {
						name += next;
						current++;
						next = _data[current];
					}

					current++;
					next = _data[current];
					std::string value = "";
					while (next != 0x0) {
						value += next;
						current++;
						next = _data[current];
					}
					current++;

					_strings[name] = value;
				}
			}

			std::string get(std::string name, std::initializer_list<std::string> args = {}) {
				// Find string
				auto it = _strings.find(name);
				if (it == _strings.end()) {
					return "(undefined)";
				}

				// Substitution
				std::string text = it->second;
				auto i = 0;
				for (auto& arg : args) {
					std::string substring = "{" + std::to_string(i) + "}";
					auto found = text.find(substring, 0);
					if (found != std::string::npos) {
						text.replace(found, substring.length(), arg);
					}
					i++;
				}

				return text;
			}
		private:
			std::unordered_map<std::string, std::string> _strings;
	};
}
