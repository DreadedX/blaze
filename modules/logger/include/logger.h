#pragma once

#include "fmt/format.h"

#include <iostream>

#if __ANDROID__
#include "android.h"
#endif

enum class Level {
	debug,
	message,
	error
};

// @todo We need to move the enviroment thing back to a seperate top level module
template <typename... Args>
void log(Level level, std::string string, Args... args) {
	std::string text = fmt::format(string, args...);
#if __ANDROID__
	java_print(text);
#else
	switch (level) {
		#ifdef DEBUG
		case Level::debug:
			std::cout << text;
			return;
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
#endif
}

