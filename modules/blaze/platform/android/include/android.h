#pragma once

#include "platform/platform.h"

namespace BLAZE_NAMESPACE::platform {
	class Android : public Platform {
		public:
			Android();

			const std::string get_base_path() const override;

			bool has_async_support() const override;
	};
}
