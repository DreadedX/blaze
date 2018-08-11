#include "flame/file_handler.h"

#include <sstream>

namespace FLAME_NAMESPACE {
	FileHandler::FileHandler(std::string filename, std::ios::openmode openmode) : _fs(filename, openmode) {
		if (!_fs.is_open()) {
			std::stringstream ss;
			ss << "Failed to open file: '" << filename << '\'';
			throw std::runtime_error(ss.str());
		}
	}

	FileHandler::~FileHandler() {
		close();
	}

	void FileHandler::close() {
		std::lock_guard<std::mutex> lock(_mutex);
		if (_fs.is_open()) {
			_fs.close();
		}
	}

	std::fstream& FileHandler::lock() {
		_mutex.lock();
		return _fs;
	}

	bool FileHandler::is_open() {
		std::lock_guard<std::mutex> lock(_mutex);
		return _fs.is_open();
	}

	void FileHandler::unlock() {
		_mutex.unlock();
	}
}
