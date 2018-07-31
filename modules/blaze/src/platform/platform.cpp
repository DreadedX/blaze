#include "platform/platform.h"

namespace BLAZE_NAMESPACE::platform {

	#if __EMSCRIPTEN__
		void dummy_async(void*) {
			LOG_D("Browser keepalive\n");
		}
		
		Web::Web() {
			// This is to make sure that Browser gets included
			emscripten_async_call(&dummy_async, 0, 0);
		}
	#endif
	}
