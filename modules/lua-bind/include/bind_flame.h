#pragma once

#include "flame.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#include "sol.hpp"
#pragma GCC diagnostic pop

namespace FLAME_NAMESPACE::lua {
	void bind(sol::state& lua);
}
