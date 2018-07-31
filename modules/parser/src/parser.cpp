#include "parser.h"

Option::Option(std::string short_name, std::string long_name) : _short_name(short_name), _long_name(long_name) {}

void Option::parse(int argc, char* argv[]) {
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

const std::string& Option::get_short() const {
	return _short_name;
}

const std::string& Option::get_long() const {
	return _long_name;
}

void Required::parse(int argc, char* argv[], int number) {
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

Parser::Parser(std::string executable) : _executable(executable) {}

void Parser::add_option(std::string name, Option option, std::string description) {
	_options.insert({name, option});
	_descriptions.insert({name, description});
}

void Parser::add_required(std::string name, Required required) {
	_required.insert({name, required});
}

void Parser::parse(int argc, char* argv[]) {
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

bool Parser::get_option(std::string name) {
	return get_option(name, false);
}

// @todo Make this prettier
void Parser::print_help() {
	LOG_M("Usage: {} [options]", _executable);
	for (auto&& required : _required) {
		LOG_M(" {}", required.first);
	}
	LOG_M("\nOptions:\n");
	for (auto&& option : _options) {
		// std::cout << "  " << std::setw(4) << std::left << option.second.get_short() << std::setw(10) << option.second.get_long() << "  " << _descriptions[option.first] << '\n';
		// @todo Fix layout
		LOG_M(" {} \t{} \t{}\n", option.second.get_short(), option.second.get_long(), _descriptions[option.first]);
	}
}
