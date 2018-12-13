#pragma once

#include "flame.h"

#include <cstdint>
#include <memory>
#include <future>
#include <vector>

namespace FLAME_NAMESPACE {
	class DataLoader {
		public:
			DataLoader(std::future<std::vector<uint8_t>> future, bool async = true);

			bool is_valid();
			bool is_loaded();

			void wait();

			std::vector<uint8_t> get();

			template <typename T>
			T get_as();

		private:
			std::future<std::vector<uint8_t>> _future;
			bool _async;
	};
};
