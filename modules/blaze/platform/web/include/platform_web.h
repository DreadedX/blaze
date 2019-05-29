#include "platform/platform.h"

#include <emscripten.h>

namespace BLAZE_NAMESPACE::platform {
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
				return _internal::std_logger;
			}
	};
}
