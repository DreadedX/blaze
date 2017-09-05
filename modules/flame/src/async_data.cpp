#include "async_data.h"

#include <iostream>

namespace blaze::flame {
	State ASyncData::get_state() {
		std::lock_guard<std::mutex> lock(_mutex);
		return _state;
	}
	bool ASyncData::is_loaded() {
		return get_state() == State::LOADED;
	}
	void ASyncData::set_state(State state) {
		std::lock_guard<std::mutex> lock(_mutex);
		_state = state;
	}
	uint32_t ASyncData::get_size() {
		_wait_until_loaded();
		return _size;
	}

	// @note Never store the result of this function, as ASyncData going out of scope deletes it
	uint8_t* ASyncData::data() {
		_wait_until_loaded();
		return _data.get();
	}

	uint8_t& ASyncData::operator[](int idx) {
#ifndef NDEBUG
		if (idx < 0 || idx >= _size) {
			std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "Array out of bounds\n";
		}
#endif
		return data()[idx];
	}

	void ASyncData::set_data(std::shared_ptr<uint8_t[]> data, uint32_t size) {
		_data = data;
		_size = size;
		set_state(State::LOADED);
		_cv.notify_all();
	}

	void ASyncData::_wait_until_loaded() {
			std::unique_lock<std::mutex> lock(_mutex);
			_cv.wait(lock, [this]{ return (_state == State::LOADED || _state == State::FAILED); });
	}
}
