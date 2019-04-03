// @note This should be the only file in the projects that has platform dependent preprocessor macros
// Everywhere else we should use constexpr
// Platform specific code should go into platform

#include "logger.h"

#include "engine.h"

#include "version.h"
#include "enviroment.h"

#include "blaze/config.h"
#include "cvar.h"

#include <fstream>

// @note This is implemtented by the user
void game();

std::ofstream log_file;

#ifdef DEBUG
std::ofstream verbose_log_file;
#endif

void file_logger(Level level, std::string file, int line, std::string text) {
	static int& log_level = blaze::CVar::get<int>("log_level");

	if (level < (Level)log_level) {
		return;
	}

	log_file << text;
	#ifdef DEBUG
		std::string level_name;
		switch (level) {
			case Level::debug:
				level_name = "debug";
				break;
			case Level::message:
				level_name = "message";
				break;
			case Level::error:
				level_name = "error";
				break;
			default:
				level_name = "unknown";
				break;
		}
		verbose_log_file << logger::prefixer(text, "\nFILE: " + file + ", LINE: " + std::to_string(line) + ", LEVEL: " + level_name + '\n');
	#endif
}

void parse_cvars(std::istream& stream) {
	std::string word;
	std::string name;

	while (stream >> word) {
		if (!name.empty()) {
			blaze::CVar::set<int>(name, std::stoi(word));

			name.clear();
		} else {
			name = word;
		}
	}
}

// This is the entry point of the game engine
int main(int argc, const char* argv[]) {
	std::ifstream cvar_file("cvars.txt", std::ios::in);

	// Loaded cvars from optional file
	if (cvar_file.is_open()) {
		parse_cvars(cvar_file);
	}

	// Load cvars from command line, overwrites those from the file
	// @todo This is very basic and does almost no checking at all
	if ((argc-1) % 2 == 0) {
		for (int i = 1; i < argc; i+=2) {
			std::cout << "arg: " << argv[i] << argv[i+1] << '\n';
			blaze::CVar::set<int>(argv[i], std::stoi(argv[i+1]));
		}
	}

	// Set the remaining cvars to their default value
	blaze::set_default_cvars();

	// try {
	if constexpr (blaze::enviroment::os == blaze::enviroment::OS::Linux) {
		blaze::set_platform<blaze::platform::Linux>();
	} else if constexpr (blaze::enviroment::os == blaze::enviroment::OS::Windows) {
		blaze::set_platform<blaze::platform::Windows>();
	} else if constexpr (blaze::enviroment::os == blaze::enviroment::OS::Android) {
		blaze::set_platform<blaze::platform::Android>();
	} else if constexpr (blaze::enviroment::os == blaze::enviroment::OS::Web) {
		blaze::set_platform<blaze::platform::Web>();
	}

	logger::add(blaze::get_platform()->get_logger());

	blaze::CVar::debug_print();

	// @todo Is this usefull on all platforms
	// On the web this is kind of useless probably...
	log_file.open(blaze::get_platform()->get_base_path() + "blaze.log", std::ios::out | std::ios::trunc);
	if (!log_file.is_open()) {
		throw std::runtime_error("Failed to create log file");
	}
	#ifdef DEBUG
	verbose_log_file.open(blaze::get_platform()->get_base_path() + "blaze.verbose.log", std::ios::out | std::ios::trunc);
	if (!log_file.is_open()) {
		throw std::runtime_error("Failed to create verbose log file");
	}
	#endif
	logger::add(file_logger);

	LOG_D("BLZNGN Version: {}\n", get_version_string().c_str());

	blaze::init();

	game();

	blaze::done();

	log_file.close();
	// } catch (std::exception& e) {
	// 	LOG_E("EXCEPTION: {}\n", e.what());
	// }
}
