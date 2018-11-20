#pragma once

#include "game_asset.h"

#include "lang.h"

namespace BLAZE_NAMESPACE {

	class Script : public GameAsset {
		public:
			Script(std::string asset_name);
			~Script();

			void update();

		private:
			void post_load() override;

			sol::environment environment;
			bool _loaded = false;
	};

	class Language : public GameAsset {
		public:
			Language(std::string asset_name);

			template <typename... Args>
			std::string get(std::string name, Args... args) {
				std::string value = _root.get_value(name);

				return fmt::format(value, args...);
			}
		private:
			void post_load();

			lang::Node _root;
	};
}
