#pragma once

#include "flame.h"

#include "archive.h"
#include "meta_asset.h"

#include "asset_data.h"

#include <array>
#include <cstdint>
#include <memory>

#include "sol/state.hpp"

std::array<uint8_t, 1217> load_private_key(std::string path);
std::shared_ptr<FLAME_NAMESPACE::FileHandler> open_file(std::string path);
std::shared_ptr<FLAME_NAMESPACE::FileHandler> open_new_file(std::string path);
void bind(sol::state& lua);
