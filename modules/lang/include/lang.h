#pragma once

#define LANG_NAMESPACE lang

#include <string>
#include <map>
#include <istream>
#include <unordered_map>
#include <sstream>

namespace LANG_NAMESPACE {
	// @todo Move this together with part of the plugin into a seperate module
	enum class type : uint8_t {
		NODE,
		KEY
	};

	class Value {
		public:
			Value() {}

			Value(std::string str, std::unordered_map<std::string, size_t> map) : _str(str), _map(map) {}
			Value(std::string str) {
				size_t begin = 0;
				size_t end = 0;
				while((begin = str.find("{{")) != std::string::npos) {
					end = str.find("}}");
					if (end == std::string::npos) {
						throw std::runtime_error("Unmatched {{");
					}

					std::string name = str.substr(begin+2, end-begin-2);
					str.erase(begin, end-begin+2);

					_map[name] = begin;
				}
				if (str.find("}}") != std::string::npos) {
					throw std::runtime_error("Unmatched }}");
				}
				_str = str;
			}

			template <typename T>
			static std::string format_s(std::string str, std::unordered_map<std::string, size_t>& map, std::string name, T replace_generic) {
				std::stringstream replace_stream;
				replace_stream << replace_generic;
				std::string replace = replace_stream.str();

				auto it = map.find(name);
				if (it == map.end()) {
					throw std::runtime_error("Unable to find: " + name);
				}

				size_t pos = it->second;
				str.insert(pos, replace);
				map.erase(it);

				size_t length = replace.length();
				for (auto& [n, p] : map) {
					if (p > pos) {
						p += length;
					}
				}

				return str;
			}

			template <typename T, typename... Args>
			static std::string format_s(std::string str, std::unordered_map<std::string, size_t>& map, std::string name, T replace, Args... args) {
				str = format_s(str, map, name, replace);
				return format_s(str, map, args...);
			}

			template <typename T, typename... Args>
			std::string format(std::string name, T replace, Args... args) {
				return format_s(_str, _map, name, replace, args...);
			}

			std::string format() {
				return _str;
			}

			const std::string& get_string() const {
				return _str;
			}

			const std::unordered_map<std::string, size_t>& get_map() const {
				return _map;
			}

		private:
			std::string _str;
			std::unordered_map<std::string, size_t> _map;
	};

	class Node {
		public:
			Value get_value(std::string name);

			std::string name;
			Value value;
			std::map<std::string, Node> children;

			int level = 0;

			Node* parent = nullptr;
	};

	Node parse_file(std::istream& in);
}
