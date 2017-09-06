#pragma once

#include "async_fstream.h"
#include "asset.h"

#include <cstdint>
#include <memory>
#include <future>
#include <vector>

// @todo Is this a good number?
#define CHUNK_SIZE 1024

namespace blaze::flame {

	enum class State : uint8_t {
		LOADING,
		LOADED,
		FAILED
	};

	class ASyncData {
		public:
			ASyncData(std::shared_ptr<ASyncFStream> afs, uint32_t size, uint32_t offset, std::vector<std::function<Asset::TaskData(Asset::TaskData)>> tasks);
			ASyncData();

			State get_state();
			bool is_loaded();
			uint32_t get_size();
			// @note Never store the result of this function, as ASyncData going out of scope deletes it
			uint8_t* data();
			uint8_t& operator[](uint32_t idx);

		private:
			State _state = State::LOADING;
			Asset::TaskData _task_data;
			std::future<Asset::TaskData> _future;

			void _wait_until_loaded();
	};
};
