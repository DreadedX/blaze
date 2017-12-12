#pragma once

#include "platform/platform.h"

void java_print(std::string text);

namespace BLAZE_NAMESPACE::platform {
	class Android : public Platform {
		public:
			const std::string get_base_path() const override;

			bool has_async_support() const override;
	};
}
