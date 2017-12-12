#include "logger.h"

#include <fstream>
#include <cstring>
#include <any>
#include <unordered_map>
#include <map>
#include <iomanip>

#include "binary_helper.h"
#include "archive.h"

#include "rsa.h"

// @todo The whole commanline parser should go into a seperate module

class Option {
	public:
		Option(std::string short_name, std::string long_name = "") : _short_name(short_name), _long_name(long_name) {}

		void parse(int argc, char* argv[]) {
			for (int i = 1; i < argc; ++i) {
				if (!_short_name.compare(argv[i]) || (_long_name.compare("") && !_long_name.compare(argv[i])) ) {
					_found = true;
					if (i+1 < argc) {
						_missing = false;
						_value = argv[i+1];
					}
				}
			}
		}

		const std::string& get_short() const {
			return _short_name;
		}

		const std::string& get_long() const {
			return _long_name;
		}

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
		void parse(int argc, char* argv[], int number) {
			int counter = 0;
			for (int i = 1; i < argc; ++i) {
				if (argv[i][0] != '-') {

					if (counter == number) {
						_found = true;
						_value = argv[i];
						return;
					}

					counter++;
				}
			}
		}

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
		Parser(std::string executable) : _executable(executable) {}

		void add_option(std::string name, Option option, std::string description) {
			_options.insert({name, option});
			_descriptions.insert({name, description});
		}

		void add_required(std::string name, Required required) {
			_required.insert({name, required});
		}

		void parse(int argc, char* argv[]) {
			int counter = 0;
			// @todo We need to remove elemts that have already been parsed from the list
			for (auto&& required : _required) {
				required.second.parse(argc, argv, counter);
				counter++;
			}

			// @todo We propably need to parse this first so the required parser can't be confused
			for (auto&& option : _options) {
				option.second.parse(argc, argv);
			}
		}

		template <typename T>
		T get_option(std::string name, T default_value) {
			auto option = _options.find(name);
			if (option != _options.end()) {
				return option->second.get_value(default_value);
			}

			// @todo Better error message
			throw std::logic_error("Unknown option");
		}

		bool get_option(std::string name) {
			return get_option(name, false);
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

		// @todo Make this prettier
		void print_help() {
			log(Level::debug, "Usage: {} [options]", _executable);
			for (auto&& required : _required) {
				log(Level::debug, " {}", required.first);
			}
			log(Level::debug, "\nOptions:\n");
			for (auto&& option : _options) {
				// std::cout << "  " << std::setw(4) << std::left << option.second.get_short() << std::setw(10) << option.second.get_long() << "  " << _descriptions[option.first] << '\n';
				// @todo Fix layout
				log(Level::debug, " {} \t{} \t{}\n", option.second.get_short(), option.second.get_long(), _descriptions[option.first]);
			}
		}

	private:
		std::map<std::string, Option> _options;
		std::map<std::string, Required> _required;
		std::unordered_map<std::string, std::string> _descriptions;

		std::string _executable;
};

int main(int argc, char* argv[]) {

	// @note This is propably going to be the API, and currently it kinda works
	Parser parser(argv[0]);

	parser.add_required("filename", Required());
	parser.add_option("size", Option("-s", "--size"), "Key size in bits");
	// parser.add_option("verbose", Option("-v"), "Print verbose output");

	try {
		parser.parse(argc, argv);

		// @todo We need to validate the commandline arguments
		int size = parser.get_option("size", 1024);
		std::string filename = parser.get_required<std::string>("filename");

		// @todo Give the user feedback
		log(Level::debug, "Generating {} bit key...\n", size);
		auto keys = crypto::generate_rsa_keys(size);
		log(Level::debug, "Key generated!\n");
		// @note We do not have to store the public key as we always use e=65537
		crypto::store(filename, keys.first);
		log(Level::debug, "Key stored in '{}'\n", filename);
	} catch(std::exception& e) {
		log(Level::error, "{}\n", e.what());
		parser.print_help();
	}
}
