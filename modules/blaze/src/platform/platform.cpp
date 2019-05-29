#include "platform/platform.h"

namespace BLAZE_NAMESPACE::platform {
	void _internal::std_logger(Level level, std::string, int, std::string text) {
		static int& log_level = CVar::get<int>("log_level");

		if (level < (Level)log_level) {
			return;
		}

		switch (level) {
			case Level::message:
				std::cout << text << std::flush;
				break;

			case Level::error:
				std::cerr << text << std::flush;
				break;

			case Level::debug:
				std::cout << text << std::flush;
				break;

			default:
				break;
		}
	}

}
