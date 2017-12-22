#pragma once

#include "blaze.h"

// @todo This can be moved into launch
namespace BLAZE_NAMESPACE::enviroment {

	enum class OS {
		Linux,
		Windows,
		Android,
		Web
	};

	// @note The order is important
	#if __EMSCRIPTEN__
		constexpr OS os = OS::Web;
	#elif _WIN32
		// @todo Just to get things started
		constexpr OS os = OS::Windows;
	#elif __ANDROID__
		constexpr OS os = OS::Android;
	#elif __linux__
		constexpr OS os = OS::Linux;
	#else
		#error "Target platform is not supported"
	#endif

	#if DEBUG
		constexpr bool debug = true;
	#elif NDEBUG
		constexpr bool debug = false;
	#else
		#error "No debug setting given"
	#endif
}
