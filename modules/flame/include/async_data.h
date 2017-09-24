#pragma once

#include "async_fstream.h"
#include "asset.h"

#include <cstdint>
#include <memory>
#include <future>
#include <vector>

namespace blaze::flame {

	enum class State : uint8_t {
		LOADING,
		LOADED,
		FAILED
	};

	class ASyncData {
		public:
			ASyncData(std::shared_ptr<ASyncFStream> afs, uint32_t size, uint32_t offset, Asset::Workflow workflow);
			ASyncData();

			State get_state();
			bool is_loaded();
			uint32_t get_size();
			// @note Never store the result of this function, as ASyncData going out of scope deletes it
			uint8_t* data();
			uint8_t& operator[](uint32_t idx);

		private:
			State _state = State::LOADING;
			uint32_t _size;
			std::unique_ptr<uint8_t[]> _data;
			std::future<std::pair<std::unique_ptr<uint8_t[]>, uint32_t>> _future;

			void _wait_until_loaded();
	};
};
