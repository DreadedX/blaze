#pragma once

#include "blaze.h"

#include "logger.h"

#include <vector>
#include <variant>
#include <string>
#include <unordered_map>

namespace BLAZE_NAMESPACE {
	class CVar {
		public:
			template <typename T>
			static T& get(std::string name) {
				auto value = _values.find(name);

				if (value != _values.end()) {
					return std::get<T>(value->second);
				}

				throw std::runtime_error("Unknown cvar");
			}

			template <typename T>
			static T& set(std::string name, T value) {
				auto existing = _values.find(name);
				if (existing == _values.end()) {
					return std::get<T>(_values.insert({name, value}).first->second);
				}

				existing->second = value;
				return std::get<T>(existing->second);
			}

			template <typename T>
			static T& set_default(std::string name, T value) {
				auto existing = _values.find(name);
				if (existing == _values.end()) {
					return std::get<T>(_values.insert({name, value}).first->second);
				}

				return std::get<T>(existing->second);
			}

			static void debug_print() {
				for (const auto& value : _values) {
					LOG_D("{} = {}\n", value.first, std::get<int>(value.second));
				}
			}

		private:
			static std::unordered_map<std::string, std::variant<int, float>> _values;
	};
}

