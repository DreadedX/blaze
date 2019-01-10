#include "blaze/config.h"

#include "cvar.h"

#include "logger.h"

namespace BLAZE_NAMESPACE {
	void set_default_cvars() {
		#if defined(DEBUG)
			CVar::set_default("debug", 1);
			CVar::set_default("log_level", (int)Level::debug);
		#else
			CVar::set_default("debug", 0);
			CVar::set_default("log_level", (int)Level::message);
		#endif

		// @todo We should propably make this better
		#if !defined(__EMSCRIPTEN__)
			CVar::set_default("backend", 1);
		#else
			CVar::set_default("backend", 0);
		#endif
	}
}
