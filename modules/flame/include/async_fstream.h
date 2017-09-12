#pragma once

#include <mutex>
#include <fstream>

namespace blaze::flame {
	// @todo Rename this function
	class ASyncFStream {
		public:
			ASyncFStream(std::string filename) : ASyncFStream(filename, std::ios::in | std::ios::out) {}
			ASyncFStream(std::string filename, std::ios::openmode openmode);
			~ASyncFStream();

			std::fstream& lock();
			bool is_open();
			void unlock();

		private:
			std::fstream _fs;
			std::mutex _mutex;
	};
};
