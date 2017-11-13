#pragma once

#include "crypto.h"

#include <vector>
#include <cstdint>

#include <iostream>

void rsa_test();

namespace CRYPTO_NAMESPACE {

	class RSA {
		public:
			RSA(std::vector<uint8_t> n, std::vector<uint8_t> d);
			std::vector<uint8_t> encrypt(std::vector<uint8_t> message);

			const std::vector<uint8_t>& get_n() const {
				return _n;
			}

			const std::vector<uint8_t>& get_d() const {
				return _d;
			}

		private:
			std::vector<uint8_t> _n;
			std::vector<uint8_t> _d;

	};

	std::pair<RSA, RSA> generate_rsa_keys(size_t b);
	void store(std::string filename, RSA key);
	RSA load(std::string filename);
}
