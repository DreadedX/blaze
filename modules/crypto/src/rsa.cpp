#include "rsa.h"

#include "logger.h"

#include <cstdint>
#include <vector>
#include <iostream>
#include <fstream>

#include <stdexcept>

#include "BigIntegerLibrary.hh"
#include "primes.h"

#include "iohelper/read.h"
#include "iohelper/base64.h"
#include "iohelper/memstream.h"

BigUnsigned power(BigUnsigned x, BigUnsigned m, BigUnsigned n) {
	BigUnsigned a = modexp(x, m, n);
	return a;
}

BigUnsigned data_to_big_unsigned(std::vector<uint8_t> data) {
	BigUnsigned x;

	std::reverse(data.begin(), data.end());

	size_t i;
	for (i = 0; i < data.size()/sizeof(BigUnsigned::Blk); ++i) {
		BigUnsigned::Blk block = 0;

		for (size_t j = 0; j < sizeof(BigUnsigned::Blk); ++j) {
			block |= ((BigUnsigned::Blk)data[i*sizeof(BigUnsigned::Blk)+j]) << (j*8);
		}

		x.setBlock(i, block);
	}

	size_t remainder = data.size() % sizeof(BigUnsigned::Blk);
	if (remainder) {
		BigUnsigned::Blk block = 0;
		for (size_t j = 0; j < remainder; ++j) {
			block |= ((BigUnsigned::Blk)data[i*sizeof(BigUnsigned::Blk)+j]) << (j*8);
		}
		x.setBlock(i, block);
	}

	return x;
}

std::vector<uint8_t> big_unsigned_to_data(const BigUnsigned& x) {
	size_t size = x.getLength();
	std::vector<uint8_t> data(size*sizeof(BigUnsigned::Blk));

	for (size_t i = 0; i < size; ++i) {
		auto block = x.getBlock(i);

		for (size_t j = 0; j < sizeof(BigUnsigned::Blk); ++j) {
			data[i*sizeof(BigUnsigned::Blk)+j] = block >> (j*8);
		}
	}

	std::reverse(data.begin(), data.end());

	return data;
}

namespace CRYPTO_NAMESPACE {

	RSA::RSA(std::vector<uint8_t> n, std::vector<uint8_t> d) : _n(n), _d(d) {}

	std::vector<uint8_t> RSA::encrypt(std::vector<uint8_t> message) {
		BigUnsigned n = data_to_big_unsigned(_n);
		BigUnsigned d = data_to_big_unsigned(_d);
		BigUnsigned m = data_to_big_unsigned(message);

		// assert(m < (n-1));
		auto encrypted = big_unsigned_to_data(power(m, d, n));

		return encrypted;
	}

	// @todo We can now only load from private files, add support for public keys
	// @todo Use the data from the file to verify everything
	std::pair<RSA, RSA> load(std::string filename) {
		std::fstream file(filename, std::ios::in | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("Failed to open file");
		}

		std::string encoded;
		while (!file.eof()) {
			std::string t;
			std::getline(file, t);
			if (t.find('-') == std::string::npos) {
				encoded += t;
			}
		}

		file.close();

		auto decoded = iohelper::base64::decode(encoded);
		auto stream = iohelper::imemstream(decoded);

		// SEQUENCE
		if (iohelper::read<uint8_t>(stream) != 0x30) { throw std::runtime_error("File not in correct format"); }

		// Length of SEQUENCE
		if (iohelper::read<uint8_t>(stream) != 0x82) { throw std::runtime_error("File not in correct format"); }
		iohelper::read<uint8_t>(stream);
		iohelper::read<uint8_t>(stream);

		// INT: Version
		if (iohelper::read<uint8_t>(stream) != 0x02) { throw std::runtime_error("File not in correct format"); }
		if (iohelper::read<uint8_t>(stream) != 0x01) { throw std::runtime_error("File not in correct format"); }
		if (iohelper::read<uint8_t>(stream) != 0x00) { throw std::runtime_error("File not in correct format"); }

		if (iohelper::read<uint8_t>(stream) != 0x02) { throw std::runtime_error("File not in correct format"); }
		auto n = iohelper::read<std::vector<uint8_t>>(stream);

		if (iohelper::read<uint8_t>(stream) != 0x02) { throw std::runtime_error("File not in correct format"); }
		auto e_temp = iohelper::read<std::vector<uint8_t>>(stream);
		std::vector<uint8_t> e;
		e.insert(e.end(), e_temp.begin(), e_temp.end());

		if (iohelper::read<uint8_t>(stream) != 0x02) { throw std::runtime_error("File not in correct format"); }
		auto d = iohelper::read<std::vector<uint8_t>>(stream);

		return std::make_pair(RSA(n, e), RSA(n, d));
	}

	std::vector<uint8_t> default_e() {
		std::vector<uint8_t> e = {0x1, 0x00, 0x01};
		return e;
	}
}

// @todo Put this somewhere else
void rsa_test() {
	logger::add(logger::std_logger);
	auto [pub, priv] = crypto::load("private.pem");

	// This is to make sure that default e works
	auto pub2 = crypto::RSA(priv.get_n(), crypto::default_e());

	std::vector<uint8_t> message = {0x10, 0, 0, 0, 0, 0, 0, 0x10, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};

	auto encrypted = priv.encrypt(message);
	auto decrypted = pub2.encrypt(encrypted);

	LOG_D("Message: ");
	for (auto&& byte : message) {
		LOG_D("{:X} ", byte);
	}
	LOG_D("\n");
	LOG_D("Encrypted: ");
	for (auto&& byte : encrypted) {
		LOG_D("{:X} ", byte);
	}
	LOG_D("\n");
	LOG_D("Size: {}\n", encrypted.size());
	LOG_D("Decrypted: ");
	for (auto&& byte : decrypted) {
		LOG_D("{:X} ", byte);
	}
	LOG_D("\n");
}
