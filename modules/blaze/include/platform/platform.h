#pragma once

#include "blaze.h"

#include "logger.h"

#include "cvar.h"

#include <string>
#include <functional>

// @todo Move each of these things into their own module and make the build system link the proper modules
// @todo Print function that calls native implemtation or something along thoses lines
// @todo Move the environment stuff in here

namespace BLAZE_NAMESPACE::platform {
	namespace _internal {
		void std_logger(Level level, std::string, int, std::string text);
	}

	class Platform {
		public:
			Platform() {}
			virtual ~Platform() {}

			virtual const std::string get_base_path() const = 0;
			virtual bool has_async_support() const = 0;
			// @todo Give this a better name
			virtual logger::LogHandler get_logger() =0;
	};

	void set();
}
