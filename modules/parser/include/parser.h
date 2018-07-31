#pragma once

#include "logger.h"

#include <cstring>
#include <any>
#include <unordered_map>
#include <map>
#include <iomanip>

class Option {
	public:
		Option(std::string short_name, std::string long_name = "");

		void parse(int argc, char* argv[]);
		const std::string& get_short() const;
		const std::string& get_long() const;

		template <typename T>
		T get_value(T default_value) {
			if constexpr (std::is_same<T, bool>()) {
				return _found;
			}

			if (_found) {
				if (_missing) {
					// @todo Better error message
					throw std::runtime_error("Missing part of option");
				}

				if constexpr (std::is_same<T, int>()) {
					return std::stoi(_value);
				} else {
					return _value;
				}
			} else {
				return default_value;
			}
		}

	private:
		std::string _short_name;
		std::string _long_name;

		bool _found = false;
		bool _missing = true;
		const char* _value;
};

class Required {
	public:
		void parse(int argc, char* argv[], int number);

		template <typename T>
		T get_value() {
			if (_found) {
				return _value;
			} else {
				// @todo Better error message
				throw std::runtime_error("Missing required");
			}
		}

	private:
		bool _found = false;
		const char* _value;
};

// @todo This can just be a namespace as we never need more than one parser
// @todo We need somehow mark everything that has been parsed, and throw an error for things that have not been parsed
// @todo Also we could now parse another options as an value
class Parser {
	public:
		Parser(std::string executable);

		void add_option(std::string name, Option option, std::string description);
		void add_required(std::string name, Required required);
		void parse(int argc, char* argv[]);
		bool get_option(std::string name);
		// @todo Make this prettier
		void print_help();

		template <typename T>
		T get_option(std::string name, T default_value) {
			auto option = _options.find(name);
			if (option != _options.end()) {
				return option->second.get_value(default_value);
			}

			// @todo Better error message
			throw std::logic_error("Unknown option");
		}

		template <typename T>
		T get_required(std::string name) {
			auto required = _required.find(name);
			if (required != _required.end()) {
				return required->second.get_value<T>();
			}

			// @todo Better error message
			throw std::logic_error("Unknown required");
		}

	private:
		std::map<std::string, Option> _options;
		std::map<std::string, Required> _required;
		std::unordered_map<std::string, std::string> _descriptions;

		std::string _executable;
};

