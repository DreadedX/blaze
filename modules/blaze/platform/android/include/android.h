#pragma once

#include "platform/platform.h"

#include <jni.h>

namespace BLAZE_NAMESPACE::platform {
	class Android : public Platform {
		public:
			const std::string get_base_path() const override;
			bool has_async_support() const override;
			logger::LogHandler get_logger() override;
	};
}
