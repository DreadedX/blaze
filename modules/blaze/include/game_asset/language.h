#pragma once

#include "game_asset.h"

#include "lang.h"

#include <fmt/format.h>

namespace BLAZE_NAMESPACE {
	class Language : public GameAsset {
		public:
			Language(std::string asset_name);

			std::string get(std::string name, std::vector<std::pair<std::string, std::string>> replaces) {
				lang::Value value = _root.get_value(name);

				std::string str = value.get_string();
				std::unordered_map<std::string, std::pair<size_t, lang::Expression>> map = value.get_map();

				for (auto& [n, r] : replaces) {
					str = lang::Value::format_s(str, map, n, r);
				}

				return str;
			}

			template <typename... Args>
			std::string get(std::string name, Args... args) {
				lang::Value value = _root.get_value(name);

				return value.format(args...);
			}

		private:
			std::vector<uint8_t> load(std::vector<uint8_t> data);

			lang::Node _root;
	};
}
