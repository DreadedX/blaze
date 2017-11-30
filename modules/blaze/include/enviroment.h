#pragma once

#include "blaze.h"

// @todo This can be moved into launch
namespace BLAZE_NAMESPACE::enviroment {

	enum class OS {
		Linux,
		// Windows,
		Android,
		Web
	};

	// @note The order is important
	#if __EMSCRIPTEN__
		constexpr OS os = OS::Web;
	#elif __ANDROID__
		constexpr OS os = OS::Android;
	#elif __linux__
		constexpr OS os = OS::Linux;
	// #elif __WIN32
	// 	constexpr OS os = OS::Windows;
	#else
		#error "Target platform is not supported"
	#endif
}
