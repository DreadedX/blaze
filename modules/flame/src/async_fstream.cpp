#include "async_fstream.h"

namespace blaze::flame {
	ASyncFStream::ASyncFStream(std::string filename, std::ios::openmode openmode) : _fs(filename, openmode) {}

	ASyncFStream::~ASyncFStream() {
		close();
	}

	void ASyncFStream::close() {
		std::lock_guard<std::mutex> lock(_mutex);
		if (_fs.is_open()) {
			_fs.close();
		}
	}

	std::fstream& ASyncFStream::lock() {
		_mutex.lock();
		return _fs;
	}

	bool ASyncFStream::is_open() {
		std::lock_guard<std::mutex> lock(_mutex);
		return _fs.is_open();
	}

	void ASyncFStream::unlock() {
		_mutex.unlock();
	}
}
