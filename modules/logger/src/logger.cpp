#include "logger.h"

std::list<std::function<void(Level, std::string)>> logger::_handlers;

void logger::add(std::function<void(Level, std::string)> handler) {
	_handlers.push_back(handler);
}

