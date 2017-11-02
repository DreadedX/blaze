#pragma once

#include "crypto.h"

#include <vector>
#include <cstdint>

namespace CRYPTO_NAMESPACE {

	class SHA3 {
		public:
			SHA3(size_t d) : _d(d), _b(1600), _c(2*_d), _r(_b-_c), _S(_b) {}

			void update(std::vector<uint8_t> data);
			std::vector<uint8_t> finalize();

			size_t digest_size() {
				return _d/8;
			}

		private:

			void process();

			size_t _d;
			size_t _b;
			size_t _c;
			size_t _r;

			std::vector<bool> _S;
			std::vector<bool> _P;
	};

	class SHA3_256 : public SHA3 {
		public:
			SHA3_256() : SHA3(256) {}
	};
}
