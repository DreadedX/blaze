#include "rsa.h"

#include <cstdint>
#include <vector>
#include <random>
#include <iostream>
#include <bitset>
#include <chrono>
#include <cassert>
#include <fstream>

#include "BigIntegerLibrary.hh"
#include "primes.h"

std::random_device generator;

BigUnsigned get_random(size_t b) {
	std::uniform_int_distribution<uint8_t> distribution(1, b/8);
	uint8_t a = distribution(generator);

	distribution = std::uniform_int_distribution<uint8_t>();

	BigUnsigned x;
	for (size_t i = 0; i < a; ++i) {
		x += BigUnsigned(distribution(generator)) << (i*8);
	}

	return x;
}

BigUnsigned generate_random_t(size_t b) {
	if ((b % 8) != 0) {
		throw std::logic_error("Amount of bits is not dividable in whole bytes");
	}

	std::uniform_int_distribution<uint8_t> distribution;

	BigUnsigned t;
	for (size_t i = 0; i < b/8; ++i) {
		t += BigUnsigned(distribution(generator)) << (i*8);
	}

	t.setBit(0, true);
	t.setBit(b-1, true);

	return t;
}

BigUnsigned generate_random_between(const BigUnsigned& lower, const BigUnsigned& upper, size_t b) {
	BigUnsigned x = get_random(b);
	while (x < lower || x > upper) {
		x = get_random(b);
		std::cout << "Iter\n";
	}

	return x;
}

bool basic_is_prime(const BigUnsigned& n) {
	if ((n % 2) == 0) {
		return n==2;
	}

	// @note We assume that n is not the prime, because these primes are way smaller than the numbers we are going to be checking
	for (auto&& p : primes) {
		if ((n % p) == 0) {
			return n==p;
		}
	}

	return true;
}

BigUnsigned power(BigUnsigned x, BigUnsigned m, BigUnsigned n) {
	BigUnsigned a = modexp(x, m, n);
	return a;
}

bool prime_test(const BigUnsigned& n) {
	if (!basic_is_prime(n)) {
		return false;
	}

	BigUnsigned m = n - 1;
	BigUnsigned s = 0;
	while ((m % 2) == 0) {
		s += 1;
		m /= 2;
	}
	std::vector<BigUnsigned> liars;

	while (liars.size() < 100) {
		BigUnsigned x = generate_random_between(2, n-1, n.bitLength());
		// @todo Make sure we have not already checked the number

		BigUnsigned xi = power(x, m, n);
		bool witness = true;

		if (xi == 1 || xi == n-1) {
			witness = false;
		} else {
			for (BigUnsigned i = 0; i < s-1; ++i) {
				xi = (xi*xi) % n;
				if (xi == 1) {
					return false;
				} else if (xi == n-1) {
					witness = false;
					break;
				}
			}
			xi = (xi*xi) % n;
			if (xi != 1) {
				return false;
			}
		}
		if (witness) {
			return false;
		} else {
			liars.push_back(x);
		}
	}

	return true;
}

BigUnsigned generate_random_prime(size_t b) {
	BigUnsigned p = generate_random_t(b);

	size_t i = 0;
	while (true) {
		if (prime_test(p)) {
			std::cout << "It took " << i <<  " iterations\n";
			return p;
		} else if (((i+1) % (2*b)) == 0) {
			p = generate_random_t(b);
		} else {
			p += 2;
		}

		i++;
	}
}

BigUnsigned data_to_big_unsigned(std::vector<uint8_t> data) {
	assert(data.size() % 8 == 0);
	BigUnsigned x;

	for (size_t i = 0; i < data.size()/8; ++i) {
		BigUnsigned::Blk block = 0;

		for (size_t j = 0; j < 8; ++j) {
			block |= ((uint64_t)data[i*8+j]) << (j*8);
		}

		x.setBlock(i, block);
	}

	std::cout << "to bu: " << x << '\n';

	return x;
}

std::vector<uint8_t> big_unsigned_to_data(const BigUnsigned& x) {
	std::cout << "from bu: " << x << '\n';
	size_t size = x.getLength();
	std::vector<uint8_t> data(size*8);

	for (size_t i = 0; i < size; ++i) {
		auto block = x.getBlock(i);

		for (size_t j = 0; j < 8; ++j) {
			data[i*8+j] = block >> (j*8);
		}
	}

	return data;
}

namespace CRYPTO_NAMESPACE {

	RSA::RSA(std::vector<uint8_t> n, std::vector<uint8_t> d) : _n(n), _d(d) {}

	std::vector<uint8_t> RSA::encrypt(std::vector<uint8_t> message) {
		BigUnsigned n = data_to_big_unsigned(_n);
		BigUnsigned d = data_to_big_unsigned(_d);
		BigUnsigned m = data_to_big_unsigned(message);

		auto encrypted = big_unsigned_to_data(power(m, d, n));

		return encrypted;
	}

	std::pair<RSA, RSA> generate_rsa_keys(size_t b) {
		BigUnsigned p = generate_random_prime(b/2);
		BigUnsigned q = generate_random_prime(b/2);
		while (p == q) {
			q = generate_random_prime(b/2);
		}
		BigUnsigned n = p * q;

		BigUnsigned phi = (p-1)*(q-1);

		BigUnsigned e = 65537;
		BigUnsigned d = modinv(e, phi);

		BigUnsigned m = 16;

		std::cout << power(m, e, n) << '\n';

		auto nv = big_unsigned_to_data(n);
		auto ev = big_unsigned_to_data(e);
		auto dv = big_unsigned_to_data(d);

		return std::make_pair(RSA(nv, dv), RSA(nv, ev));
	}


	void store(std::string filename, RSA key) {
		std::fstream file(filename, std::ios::out | std::ios::trunc);

		auto n = key.get_n();
		auto d = key.get_d();

		// @todo Use the binary helper, after moving it to it's own module
		for (size_t i = 0; i < sizeof(uint16_t); ++i) {
			uint8_t byte = ((n.size() >> i*8) & 0xff);
			file.write(reinterpret_cast<const char*>(&byte), 1);
		}
		file.write(reinterpret_cast<const char*>(n.data()), n.size());

		for (size_t i = 0; i < sizeof(uint16_t); ++i) {
			uint8_t byte = ((d.size() >> i*8) & 0xff);
			file.write(reinterpret_cast<const char*>(&byte), 1);
		}
		file.write(reinterpret_cast<const char*>(d.data()), d.size());
	}

	RSA load(std::string filename) {
		std::fstream file(filename, std::ios::in);

		// @todo Verify the file somehow
		uint16_t size = 0;
		for (size_t i = 0; i < sizeof(uint16_t); ++i) {
			uint8_t byte;
			file.read(reinterpret_cast<char*>(&byte), 1);
			size |= (byte << i*8);
		}
		std::vector<uint8_t> n(size);
		file.read(reinterpret_cast<char*>(n.data()), size);

		size = 0;
		for (size_t i = 0; i < sizeof(uint16_t); ++i) {
			uint8_t byte;
			file.read(reinterpret_cast<char*>(&byte), 1);
			size |= (byte << i*8);
		}
		std::vector<uint8_t> d(size);
		file.read(reinterpret_cast<char*>(d.data()), size);

		return RSA(n, d);
	}
}

void rsa_test() {
	{
		auto keys = crypto::generate_rsa_keys(1024);

		crypto::store("test.priv", keys.first);
		// crypto::store("test.pub", keys.second);
	}

	{
		auto priv = crypto::load("test.priv");
		auto pub = crypto::load("test.pub");

		std::vector<uint8_t> message = {0x10, 0, 0, 0, 0, 0, 0, 0};

		auto encrypted = priv.encrypt(message);
		auto decrypted = pub.encrypt(encrypted);

		std::cout << "Message: ";
		for (auto&& byte : message) {
			std::cout << std::hex << (uint32_t)byte << ' ';
		}
		std::cout << '\n';
		std::cout << "Encrypted: ";
		for (auto&& byte : encrypted) {
			std::cout << std::hex << (uint32_t)byte << ' ';
		}
		std::cout << "Decrypted: ";
		for (auto&& byte : decrypted) {
			std::cout << std::hex << (uint32_t)byte << ' ';
		}
		std::cout << '\n';
	}
}
