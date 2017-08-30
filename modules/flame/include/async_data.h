#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <cassert>

namespace blaze::flame {

	enum class State : uint8_t {
		NOT_LOADED,
		LOADING,
		LOADED
	};

	class ASyncData {
		public:
			const State get_state() {
				std::lock_guard<std::mutex> lock(_mutex);
				return _state;
			}
			const bool is_loaded() {
				return get_state() == State::LOADED;
			}
			void set_state(State state) {
				std::lock_guard<std::mutex> lock(_mutex);
				_state = state;
			}
			const uint32_t get_size() {
				{
					std::unique_lock<std::mutex> lock(_mutex);
					_cv.wait(lock, [this]{ return (_state == State::LOADED); });
					assert(_data);
				}
				return _size;
			}

			// @note Never store the result of this function, as ASyncData going out of scope deletes it
			uint8_t* data() {
				{
					std::unique_lock<std::mutex> lock(_mutex);
					_cv.wait(lock, [this]{ return (_state == State::LOADED); });
					assert(_data);
				}
				return _data.get();
			}

			uint8_t& operator[](int idx) {
				{
					std::unique_lock<std::mutex> lock(_mutex);
					_cv.wait(lock, [this]{ return (_state == State::LOADED); });
					assert(_data);
				}
				#ifndef NDEBUG
					if (idx < 0 || idx >= _size) {
						std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "Array out of bounds\n";
					}
				#endif
				return _data[idx];
			}

			void set_data(std::shared_ptr<uint8_t[]> data, uint32_t size) {
				_data = data;
				_size = size;
				set_state(State::LOADED);
				_cv.notify_all();
			}

		private:
			State _state = State::NOT_LOADED;
			// Size of the data stream
			uint32_t _size = 0;
			std::shared_ptr<uint8_t[]> _data = nullptr;
			std::mutex _mutex;
			std::condition_variable _cv;
	};
};
