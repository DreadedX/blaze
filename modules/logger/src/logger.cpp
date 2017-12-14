#include "logger.h"

std::list<std::function<void(Level, std::string)>> logger::_handlers;

void logger::add(std::function<void(Level, std::string)> handler) {
	_handlers.push_back(handler);
}

void logger::std_logger(Level level, std::string text) {
	switch (level) {
		#ifdef DEBUG
			case Level::debug:
				std::cout << text;
				break;
		#endif

		case Level::message:
			std::cout << text;
			break;

		case Level::error:
			std::cerr << text;
			return;

		default:
			break;
	}
}

