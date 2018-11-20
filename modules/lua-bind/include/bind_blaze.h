#pragma once

#include "blaze.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#pragma warning(push, 0)
#include "sol.hpp"
#pragma GCC diagnostic pop
#pragma warning(pop)

namespace BLAZE_NAMESPACE::lua {
	void bind(sol::state& lua);
}
