#pragma once

#include "blaze.h"

#include "asset_list.h"
#include "events.h"

#include "binary_helper.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#include "sol/state.hpp"
#pragma GCC diagnostic pop

#include "game_asset.h"

#include <initializer_list>
#include <string>
#include <variant>

namespace BLAZE_NAMESPACE {
	void initialize(std::initializer_list<std::string> archives);
	void load_archive(std::string archive_name);
	void update();
	sol::state& get_lua_state();
}
