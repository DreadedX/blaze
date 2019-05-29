#pragma once

#define LANG_NAMESPACE lang

#include <string>
#include <map>
#include <iostream>
#include <unordered_map>
#include <sstream>
#include <vector>

#include <cctype>

namespace LANG_NAMESPACE {
	// @todo Move this together with part of the plugin into a seperate module
	enum class type : uint8_t {
		NODE,
		KEY
	};

	class Expression {
		public:
			enum OP : uint8_t {
				EQUALS,
				LARGER,
				SMALLER,
				LEQUALS,
				SEQUALS,
				NOT,
				DEFAULT
			};

			Expression(std::string expression) {
				expression += '\0';
				bool open = false;

				std::string temp;
				std::string criteria_string ;
				std::string op_string;
				for (auto& c : expression) {
					if (c == '"') {
						open = !open;
					}

					if (open) {
						temp += c;
					} else {
						if (c == '>' || c == '<' || c == '=' || c == '~') {
							op_string += c;
						} else if (c == '-' || isdigit(c)) {
							criteria_string += c;
						} else if (c == ',' || c == '\0') {
							temp = temp.substr(1);
							if (op_string.empty() && criteria_string.empty()) {
								_ops.push_back({OP::DEFAULT, 0, temp});
							} else if (!op_string.empty() && !criteria_string.empty()) {
								if (!op_string.compare("==")) {
									_ops.emplace_back(OP::EQUALS, std::stoi(criteria_string), temp);
								} else if (!op_string.compare(">")) {
									_ops.emplace_back(OP::LARGER, std::stoi(criteria_string), temp);
								} else if (!op_string.compare("<")) {
									_ops.emplace_back(OP::SMALLER, std::stoi(criteria_string), temp);
								} else if (!op_string.compare(">=")) {
									_ops.emplace_back(OP::LEQUALS, std::stoi(criteria_string), temp);
								} else if (!op_string.compare("<=")) {
									_ops.emplace_back(OP::SEQUALS, std::stoi(criteria_string), temp);
								} else if (!op_string.compare("~=")) {
									_ops.emplace_back(OP::NOT, std::stoi(criteria_string), temp);
								} else {
									throw std::runtime_error("Unknown operator");
								}
							} else {
								// @todo Improve error reporting
								throw std::runtime_error("Malformed expression");
							}

							temp.clear();
							criteria_string.clear();
							op_string.clear();
						}
					}
				}
				if (open) {
					throw std::runtime_error("Unmatched \"");
				}
			}

			Expression(std::vector<std::tuple<OP, int32_t, std::string>> ops) : _ops(ops) {}

			Expression() {}

			template <typename T>
			std::string get(T value) {
				if (typeid(T) == typeid(std::string)) {
					try {
						int32_t v = std::stoi(value);
						return get(v);
					} catch (std::invalid_argument& e) {

					}
				}
				if (_ops.size() == 0) {
					std::stringstream stream;
					stream << value;
					return stream.str();
				} else {
					throw std::runtime_error("Complex expression requires int");
				}
			}

			std::string get(int32_t value) {
				if (_ops.size() == 0) {
					std::stringstream stream;
					stream << value;
					return stream.str();
				}

				std::string def = "";
				for (auto& [op, i, v] : _ops) {
					switch (op) {
						case OP::EQUALS:
							if (value == i) {
								return v;
							}
							break;
						case OP::LARGER:
							if (value > i) {
								return v;
							}
							break;
						case OP::SMALLER:
							if (value < i) {
								return v;
							}
							break;
						case OP::LEQUALS:
							if (value >= i) {
								return v;
							}
							break;
						case OP::SEQUALS:
							if (value <= i) {
								return v;
							}
							break;
						case OP::NOT:
							if (value != i) {
								return v;
							}
							break;
						case OP::DEFAULT:
							def = v;
							break;
					}
				}

				return def;
			}

			const std::vector<std::tuple<OP, int32_t, std::string>>& get_ops() const {
				return _ops;
			}

		private:
			std::vector<std::tuple<OP, int32_t, std::string>> _ops;
	};

	class Value {
		public:
			Value() {}

			Value(std::string str, std::unordered_map<std::string, std::pair<size_t, Expression>> map) : _str(str), _map(map) {}
			Value(std::string str) {
				size_t begin = 0;
				while((begin = str.find("{{")) != std::string::npos) {
					size_t end = str.find("}}");
					if (end == std::string::npos) {
						throw std::runtime_error("Unmatched {{");
					}

					std::string name = str.substr(begin+2, end-begin-2);
					str.erase(begin, end-begin+2);

					end = name.find(":");
					if (end == std::string::npos) {
						_map[name] = std::make_pair(begin, Expression());
					} else {
						std::string expression = name.substr(end+1);
						name = name.substr(0, end);
						_map[name] = std::make_pair(begin, Expression(expression));
					}

				}
				if (str.find("}}") != std::string::npos) {
					throw std::runtime_error("Unmatched }}");
				}
				_str = str;
			}

			template <typename T>
			static std::string format_s(std::string str, std::unordered_map<std::string, std::pair<size_t, Expression>>& map, std::string name, T replace_generic) {
				auto it = map.find(name);
				if (it == map.end()) {
					throw std::runtime_error("Unable to find: " + name);
				}

				size_t pos = it->second.first;
				std::string replace = it->second.second.get(replace_generic);
				str.insert(pos, replace);
				map.erase(it);

				size_t length = replace.length();
				for (auto& [n, p] : map) {
					if (p.first > pos) {
						p.first += length;
					}
				}

				return str;
			}

			template <typename T, typename... Args>
			static std::string format_s(std::string str, std::unordered_map<std::string, std::pair<size_t, Expression>>& map, std::string name, T replace, Args... args) {
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

			const std::unordered_map<std::string, std::pair<size_t, Expression>>& get_map() const {
				return _map;
			}

		private:
			std::string _str;
			std::unordered_map<std::string, std::pair<size_t, Expression>> _map;
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
