// @note This should be the only file in the projects that has platform dependent preprocessor macros
// Everywhere else we should use constexpr
// Platform specific code should go into platform

#include "engine.h"
#include "platform/platform.h"

#include "version.h"
#include "enviroment.h"

#include <iostream>

// @note The user has to implement this
void game();

// @note We need this because Android uses a different entrypoint
// @todo Move this to platform
#if ANDROID
	#include <jni.h>

	extern "C" {
		JNIEXPORT void JNICALL Java_nl_mtgames_blazebootstrap_BootstrapActivity_start(JNIEnv* env, jobject thiz) {
			main();
		}
	}
#endif

// This is the entry point of the game engine
int main() {
	if constexpr (enviroment::os == enviroment::OS::Linux) {
		blaze::set_platform<blaze::Linux>();
	} else if constexpr (enviroment::os == enviroment::OS::Android) {
		blaze::set_platform<blaze::Android>();
	} else if constexpr (enviroment::os == enviroment::OS::Web) {
		blaze::set_platform<blaze::Web>();
	}

	std::cout << "BLZNGN Version: " << get_version_number() << '-' << get_version_string() <<'\n';

	blaze::initialize();

	game();
}

