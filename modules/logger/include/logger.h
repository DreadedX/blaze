#pragma once

#include "fmt/format.h"

#include <iostream>
#include <functional>
#include <list>

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

		static void std_logger(Level level, std::string text);
	
	private:
		static std::list<std::function<void(Level, std::string)>> _handlers;
};

#define LOG_D(...) logger::log(Level::debug, __VA_ARGS__)
#define LOG_M(...) logger::log(Level::message, __VA_ARGS__)
#define LOG_E(...) logger::log(Level::error, __VA_ARGS__)

// template <typename... Args>
// void log(Level level, std::string format, Args... args) {
// 	logger::log(level, format, args...);
// }
