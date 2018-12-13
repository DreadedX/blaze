#pragma once

#include "flame.h"

#include "flame/meta_asset.h"

#include <cstdint>
#include <memory>
#include <future>
#include <vector>

namespace FLAME_NAMESPACE {

	class AssetData {
		public:
			AssetData(std::string filename, size_t size, size_t offset, std::vector<MetaAsset::Task> workflow, bool async = true, std::function<void(std::vector<uint8_t>)> callback = nullptr);

			bool is_loaded();
			size_t get_size();
			// @todo Redo this stuff
			template <typename T>
			T as();
			uint8_t& operator[](uint32_t idx);


		private:
			void _wait_until_loaded();

			bool _async = true;
			bool _loaded = false;
			std::vector<uint8_t> _data;
			std::future<std::vector<uint8_t>> _future;

	};
};
