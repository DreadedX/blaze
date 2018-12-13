#pragma once

#include "game_asset.h"

#include "lang.h"

#include <atomic>

namespace BLAZE_NAMESPACE {

	class Script : public GameAssetLoaded {
		public:
			Script(std::string asset_name);
			~Script();

			bool is_loaded() override;

			void update();

		private:
			sol::environment environment;
			bool _loaded = false;
	};

	class Language : public GameAssetLoaded {
		public:
			Language(std::string asset_name);

			template <typename... Args>
			std::string get(std::string name, Args... args) {
				std::string value = _root.get_value(name);

				return fmt::format(value, args...);
			}
		private:
			std::vector<uint8_t> load(std::vector<uint8_t> data);

			lang::Node _root;
	};
}
