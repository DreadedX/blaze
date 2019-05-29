#include "platform_default.h"
#include "engine.h"

namespace BLAZE_NAMESPACE::platform {
	void set() {
		blaze::set_platform<Desktop>();
	}
}
