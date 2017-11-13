#pragma once

#include "flame.h"

#include "archive.h"
#include "meta_asset.h"

#include "asset_data.h"

#include "rsa.h"

#include <array>
#include <cstdint>
#include <memory>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#include "sol/state.hpp"
#pragma GCC diagnostic pop

crypto::RSA load_private_key(std::string path);
std::shared_ptr<FLAME_NAMESPACE::FileHandler> open_file(std::string path);
std::shared_ptr<FLAME_NAMESPACE::FileHandler> open_new_file(std::string path);
void bind(sol::state& lua);
