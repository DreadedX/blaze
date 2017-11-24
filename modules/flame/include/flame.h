#pragma once

#define FLAME_NAMESPACE flame

namespace FLAME_NAMESPACE::enviroment {
#if __EMSCRIPTEN__ || NO_ASYNC
	constexpr bool async = false;
#else
	constexpr bool async = true;
#endif
}

