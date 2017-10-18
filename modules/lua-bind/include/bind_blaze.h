#pragma once

#include "blaze.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#include "sol.hpp"
#pragma GCC diagnostic pop

namespace BLAZE_NAMESPACE::lua {
	void bind(sol::state& lua);
}
