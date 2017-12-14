#pragma once

#include "blaze.h"

#include "logger.h"

#include <string>
#include <functional>

#if __ANDROID__
	#include <android/log.h>
#endif

// @todo Move each of these things into their own module and make the build system link the proper modules
// @todo Print function that calls native implemtation or something along thoses lines

namespace BLAZE_NAMESPACE::platform {

	class Platform {
		public:
			virtual const std::string get_base_path() const = 0;
			virtual bool has_async_support() const = 0;
			virtual std::function<void(Level, std::string)> logger() =0;
	};

#if __linux__
	class Linux : public Platform {
		public:
			const std::string get_base_path() const override {
				return "archives/";
			}

			bool has_async_support() const override {
				return true;
			}

			std::function<void(Level, std::string)> logger() override {
				return [](Level level, std::string text) {
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
				};
			}
	};
#else
	class Linux : public Platform {};
#endif

#if __EMSCRIPTEN__
	class Web : public Platform {
		public:
			const std::string get_base_path() const override {
				return "build/archives/";
			}

			bool has_async_support() const override{
				return false;
			}

			std::function<void(Level, std::string)> logger() override {
				return [](Level level, std::string text) {
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
				};
			}
	};
#else
	class Web : public Platform {};
#endif
}
