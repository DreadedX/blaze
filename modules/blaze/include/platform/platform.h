#pragma once

#include "blaze.h"

#include "logger.h"

#include <string>
#include <functional>

#if __EMSCRIPTEN__
	#include <emscripten.h>
#endif

// @todo Move each of these things into their own module and make the build system link the proper modules
// @todo Print function that calls native implemtation or something along thoses lines
// @todo Move the environment stuff in here

namespace BLAZE_NAMESPACE::platform {
	class Platform {
		public:
			Platform() {}
			virtual ~Platform() {}

			virtual const std::string get_base_path() const = 0;
			virtual bool has_async_support() const = 0;
			// @todo Give this a better name
			virtual logger::LogHandler get_logger() =0;
	};

#ifdef __linux__
	class Linux : public Platform {
		public:
			const std::string get_base_path() const override {
				return "./";
			}

			bool has_async_support() const override {
				return true;
			}

			logger::LogHandler get_logger() override {
				return logger::std_logger;
			}
	};
#else
	class Linux : public Platform {};
#endif

#ifdef _WIN32
	class Windows : public Platform {
		public:
			const std::string get_base_path() const override {
				return "./";
			}

			bool has_async_support() const override {
				return true;
			}

			logger::LogHandler get_logger() override {
				return logger::std_logger;
			}
	};
#else
	class Windows : public Platform {};
#endif

#ifdef __EMSCRIPTEN__
	class Web : public Platform {
		public:
			Web();

			const std::string get_base_path() const override {
				return "/data/";
			}

			bool has_async_support() const override{
				return false;
			}

			logger::LogHandler get_logger() override {
				return logger::std_logger;
			}
	};
#else
	class Web : public Platform {};
#endif
}

#ifdef __ANDROID__
	#include "android.h"
#else
namespace BLAZE_NAMESPACE::platform {
	class Android : public Platform {};
}
#endif
