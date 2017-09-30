#pragma once

#include "flame.h"

#include "file_handler.h"
#include "meta_asset.h"

#include <cstdint>
#include <memory>
#include <future>
#include <vector>

namespace FLAME_NAMESPACE {

	class AssetData {
		public:
			AssetData(std::shared_ptr<FileHandler> fh, uint32_t size, uint32_t offset, MetaAsset::Workflow workflow);

			bool is_loaded();
			uint32_t get_size();
			// @note Never store the result of this function, as AssetData going out of scope deletes it
			uint8_t* data();
			uint8_t& operator[](uint32_t idx);

		private:
			bool _loaded = false;
			uint32_t _size;
			std::unique_ptr<uint8_t[]> _data;
			std::future<std::pair<std::unique_ptr<uint8_t[]>, uint32_t>> _future;

			void _wait_until_loaded();
	};
};
