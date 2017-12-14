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

std::ofstream log_file;

void file_logger(Level, std::string text) {
	log_file << text;
}

// This is the entry point of the game engine
int main() {
	if constexpr (blaze::enviroment::os == blaze::enviroment::OS::Linux) {
		blaze::set_platform<blaze::platform::Linux>();
	} else if constexpr (blaze::enviroment::os == blaze::enviroment::OS::Android) {
		blaze::set_platform<blaze::platform::Android>();
	} else if constexpr (blaze::enviroment::os == blaze::enviroment::OS::Web) {
		blaze::set_platform<blaze::platform::Web>();
	}

	logger::add(blaze::get_platform()->logger());

	// @todo This works on web somehow???
	log_file.open("game.log", std::ios::out | std::ios::trunc);
	if (!log_file.is_open()) {
		throw std::runtime_error("Failed to create log file");
	}
	logger::add(file_logger);

	LOG_D("BLZNGN Version: {}\n", get_version_string().c_str());

	blaze::init();

	game();

	blaze::done();

	log_file.close();
}
