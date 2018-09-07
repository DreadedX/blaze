#pragma once

#include "crypto.h"

#include <vector>
#include <cstdint>

#include <iostream>

void rsa_test();

namespace CRYPTO_NAMESPACE {

	std::vector<uint8_t> default_e();

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

	std::vector<uint8_t> default_e();

	std::pair<RSA, RSA> load(std::string filename);
}
