#pragma once

#include "fmt/format.h"

#include <iostream>
#include <functional>
#include <list>

#if __ANDROID__
#include "android.h"
#endif

enum class Level {
	debug,
	message,
	error
};

class logger {
	public:
		static void add(std::function<void(Level, std::string)> handler);
		template <typename... Args>
		static void log(Level level, std::string format, Args... args) {
			for (auto&& handler : _handlers) {
				std::string text = fmt::format(format, args...);
				handler(level, text);
			}
		}
	
	private:
		static std::list<std::function<void(Level, std::string)>> _handlers;
};

template <typename... Args>
void log(Level level, std::string format, Args... args) {
	logger::log(level, format, args...);
}

// @todo We need to move the enviroment thing back to a seperate top level module
// template <typename... Args>
// void log(Level level, std::string string, Args... args) {
// 	std::string text = fmt::format(string, args...);
// #if __ANDROID__
// 	java_print(text);
// #else
// 	switch (level) {
// 		#ifdef DEBUG
// 		case Level::debug:
// 			std::cout << text;
// 			break;
// 		#endif
// 		
// 		case Level::message:
// 			std::cout << text;
// 			break;
//
// 		case Level::error:
// 			std::cerr << text;
// 			return;
//
// 		default:
// 			break;
// 	}
// #endif
// }
//
