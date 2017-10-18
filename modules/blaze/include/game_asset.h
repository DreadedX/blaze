#pragma once

#include "blaze.h"

#include "asset_data.h"
#include "asset_list.h"
#include "engine.h"

#include <variant>

namespace BLAZE_NAMESPACE {

	class GameAsset;
	// @todo This should be part of asset_manager, but than friendship does not work properly
	bool finish_load(std::shared_ptr<GameAsset> asset);
	#define MAKE_GAME_ASSET friend bool finish_load(std::shared_ptr<GameAsset> asset);

	class GameAsset {
		public:
			GameAsset(std::string asset_name);
			virtual ~GameAsset() {}

			bool is_loaded();

		protected:
			flame::AssetData _data;
			
		private:
			virtual void post_load() = 0;

		MAKE_GAME_ASSET
	};


	class Script : public GameAsset {
		public:
			Script(std::string asset_name);
			~Script();

			void update();

		private:
			void post_load() override;

			sol::environment environment;
			bool _loaded = false;

		MAKE_GAME_ASSET
	};

	typedef std::variant<std::string, int> SupportedTypes;
	std::string to_string(const sol::stack_proxy& value);
	std::string to_string(const SupportedTypes& value);

	class Language : public GameAsset {
		public:
			Language(std::string asset_name);

			template <typename T>
			std::string get(std::string name, T args) {
				// Find string
				auto it = _strings.find(name);
				if (it == _strings.end()) {
					return "(undefined)";
				}

				// Substitution
				std::string text = it->second;
				auto i = 0;
				for (const auto& arg : args) {
					// @todo Add more formatting options
					std::string substring = "{" + std::to_string(i) + "}";
					auto found = text.find(substring, 0);
					if (found != std::string::npos) {
						text.replace(found, substring.length(), to_string(arg));
					} else {
						throw std::runtime_error("Too many substitution arguments");
					}
					i++;
				}

				return text;
			}
			std::string get(std::string name);
			std::string get(std::string name, std::initializer_list<SupportedTypes> args);


		private:
			void post_load();

			std::unordered_map<std::string, std::string> _strings;

		MAKE_GAME_ASSET
	};
}
