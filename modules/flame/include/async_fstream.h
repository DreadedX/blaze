#pragma once

#include <mutex>
#include <fstream>

namespace blaze::flame {
	class ASyncFStream {
		public:
			ASyncFStream(std::string filename, std::ios::openmode openmode) : _fs(filename, openmode) {}

			~ASyncFStream() {
				std::lock_guard<std::mutex> lock(_mutex);
				_fs.close();
			}

			std::fstream& lock() {
				_mutex.lock();
				return _fs;
			}

			bool is_open() {
				std::lock_guard<std::mutex> lock(_mutex);
				return _fs.is_open();
			}

			void unlock() {
				_mutex.unlock();
			}
		private:
			std::fstream _fs;
			std::mutex _mutex;
	};
};
