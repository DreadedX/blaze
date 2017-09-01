#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <condition_variable>

namespace blaze::flame {

	enum class State : uint8_t {
		NOT_LOADED,
		LOADING,
		LOADED,
		FAILED
	};

	class ASyncData {
		public:
			const State get_state();
			const bool is_loaded();
			void set_state(State state);
			const uint32_t get_size();
			// @note Never store the result of this function, as ASyncData going out of scope deletes it
			uint8_t* data();
			uint8_t& operator[](int idx);
			void set_data(std::shared_ptr<uint8_t[]> data, uint32_t size);

		private:
			State _state = State::NOT_LOADED;
			// Size of the data stream
			uint32_t _size = 0;
			std::shared_ptr<uint8_t[]> _data = nullptr;
			std::mutex _mutex;
			std::condition_variable _cv;

			void _wait_until_loaded();
	};
};
