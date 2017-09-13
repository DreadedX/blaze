#pragma once

#include <mutex>
#include <fstream>

namespace blaze::flame {
	// @todo Rename this function
	class ASyncFStream {
		public:
			ASyncFStream(std::string filename, std::ios::openmode openmode);
			~ASyncFStream();

			void close();
			std::fstream& lock();
			bool is_open();
			void unlock();

		private:
			std::fstream _fs;
			std::mutex _mutex;
	};
};
