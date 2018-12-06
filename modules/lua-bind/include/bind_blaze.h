#pragma once

#include "blaze.h"

#if defined(__GNUC__) || defined (__clang__)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#else
	#pragma warning(push, 0)
#endif
#include "sol.hpp"
#if defined(__GNUC__) || defined (__clang__)
	#pragma GCC diagnostic pop
#else
	#pragma warning(pop)
#endif

namespace BLAZE_NAMESPACE::lua {
	void bind(sol::state& lua);
}
