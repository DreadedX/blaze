#pragma once

#include "game_asset.h"

#include "lang.h"

#include <fmt/format.h>

namespace BLAZE_NAMESPACE {
	class Language : public GameAsset {
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
