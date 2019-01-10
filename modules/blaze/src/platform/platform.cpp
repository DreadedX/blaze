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

	#if __EMSCRIPTEN__
		void dummy_async(void*) {
			LOG_D("Browser keepalive\n");
		}
		
		Web::Web() {
			// This is to make sure that Browser gets included
			emscripten_async_call(&dummy_async, 0, 0);
		}
	#endif
}
