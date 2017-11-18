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
			AssetData(std::shared_ptr<FileHandler> fh, uint32_t size, uint32_t offset, std::vector<MetaAsset::Task> workflow);

			bool is_loaded();
			uint32_t get_size();
			template <typename T>
			T as();
			uint8_t& operator[](uint32_t idx);


		private:
			void _wait_until_loaded();

			bool _loaded = false;
			std::vector<uint8_t> _data;
			std::future<std::vector<uint8_t>> _future;

	};
};
