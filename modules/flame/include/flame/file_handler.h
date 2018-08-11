#pragma once

#include "flame.h"

#include <mutex>
#include <fstream>

namespace FLAME_NAMESPACE {
	// @todo Rename this function
	class FileHandler {
		public:
			FileHandler(std::string filename, std::ios::openmode openmode);
			~FileHandler();

			void close();
			std::fstream& lock();
			bool is_open();
			void unlock();

		private:
			std::fstream _fs;
			std::mutex _mutex;
	};
};
