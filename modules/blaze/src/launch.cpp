// @note This should be the only file in the projects that has platform dependent preprocessor macros
// Everywhere else we should use constexpr
// Platform specific code should go into platform

#include "logger.h"

#include "engine.h"
#include "android.h"

#include "version.h"
#include "enviroment.h"

// @note This is implemtented by the user
void game();

// This is the entry point of the game engine
int main() {
	if constexpr (blaze::enviroment::os == blaze::enviroment::OS::Linux) {
		blaze::set_platform<blaze::platform::Linux>();
	} else if constexpr (blaze::enviroment::os == blaze::enviroment::OS::Android) {
		blaze::set_platform<blaze::platform::Android>();
	} else if constexpr (blaze::enviroment::os == blaze::enviroment::OS::Web) {
		blaze::set_platform<blaze::platform::Web>();
	}

	log(Level::debug, "BLZNGN Version: {}-{}\n", get_version_number(), get_version_string().c_str());

	blaze::initialize();

	game();
}
