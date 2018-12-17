#pragma once

#include "logger.h"

#include "blaze.h"

#include "flame/file_handle.h"

#include <string>
#include <string_view>
#include <functional>

namespace BLAZE_NAMESPACE {

	class GameAsset {
		public:
			GameAsset(std::string asset_name, std::vector<flame::FileHandle::Task> tasks = std::vector<flame::FileHandle::Task>());
			virtual ~GameAsset() {}

			virtual bool is_loaded();

			std::string_view get_name() const;

		protected:
			std::string _name;
			flame::DataHandle _data_handle;
	};
}
