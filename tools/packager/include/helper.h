#pragma once

#include "archive.h"
#include "asset.h"

#include "async_data.h"

#include <array>
#include <cstdint>
#include <memory>

#include "sol/state.hpp"

std::array<uint8_t, 1217> load_private_key(std::string path);
std::shared_ptr<blaze::flame::ASyncFStream> open_file(std::string path);
std::shared_ptr<blaze::flame::ASyncFStream> open_new_file(std::string path);
void bind(sol::state& lua);
