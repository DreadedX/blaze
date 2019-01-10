#include "blaze/config.h"

#include "cvar.h"

#include "logger.h"

namespace BLAZE_NAMESPACE {
	void set_default_cvars() {
		#if defined(DEBUG)
			int& debug_cvar = CVar::set_default("debug", 1);
		#else
			int& debug_cvar = CVar::set_default("debug", 0);
		#endif

		if (debug_cvar) {
			CVar::set_default("log_level", (int)Level::debug);
		} else {
			CVar::set_default("log_level", (int)Level::message);
		}

		// @todo We should propably make this better
		#if !defined(__EMSCRIPTEN__)
			CVar::set_default("backend", 1);
		#else
			CVar::set_default("backend", 0);
		#endif
	}
}
