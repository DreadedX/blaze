#pragma once

#include "blaze.h"

#include "asset_list.h"
#include "events.h"
#include "platform/platform.h"

#include "flame/binary_helper.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#include "sol/state.hpp"
#pragma GCC diagnostic pop

#include "game_asset.h"

#include "fmt/format.h"

#include <initializer_list>
#include <string>

namespace BLAZE_NAMESPACE {

	extern std::unique_ptr<platform::Platform> current_platform;

	template<typename T>
	void set_platform() {
		current_platform = std::make_unique<T>();
	}

	void init();
	void load_archive(std::string archive_name);
	void update();
	void done();

	sol::state& get_lua_state();
	std::unique_ptr<platform::Platform>& get_platform();
}
