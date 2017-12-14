#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "data.h"

#include "sha3.h"
#include "rsa.h"
#include <sstream>

#include "logger.h"

#include <iomanip>

// @note We are assuming that these functions are correct

std::string bytes_to_string(std::vector<uint8_t> data) {
	std::stringstream ss;
	for (auto&& d : data) {
		ss << std::setfill('0') << std::setw(2) << std::hex << (uint32_t)d;
	}

	return ss.str();
}

// All data in one go
std::string calculate_hash(std::vector<uint8_t> data) {
	crypto::SHA3_256 hash;
	hash.update(data);
	std::vector<uint8_t> digest = hash.finalize();

	std::stringstream ss;
	for (auto&& d : digest) {
		ss << std::setfill('0') << std::setw(2) << std::hex << (uint32_t)d;
	}

	return bytes_to_string(digest);
}

// Data in chunks of 211 bytes
std::string calculate_hash_multi(std::vector<uint8_t> data) {
	crypto::SHA3_256 hash;
	std::vector<uint8_t> temp;
	for (size_t i = 0; i < data.size(); ++i) {
		temp.push_back(data[i]);
		if ((i+1) % 211 == 0) {
			hash.update(temp);
			temp.clear();
		}
	}
	hash.update(temp);
	std::vector<uint8_t> digest = hash.finalize();

	std::stringstream ss;
	for (auto&& d : digest) {
		ss << std::setfill('0') << std::setw(2) << std::hex << (uint32_t)d;
	}

	return ss.str();
}

TEST_CASE( "install logger" ) {
	logger::add(logger::std_logger);
}

TEST_CASE( "sha3 hashes are calculated", "[sha3]" ) {
	SECTION( "sha3-256 empty" ) {
		REQUIRE(calculate_hash(empty) == empty_hash_256);
	}
	SECTION( "sha3-256 abc" ) {
		REQUIRE(calculate_hash(abc) == abc_hash_256);
	}
	SECTION( "sha3-256 hello" ) {
		REQUIRE(calculate_hash(hello) == hello_hash_256);
	}
	SECTION( "sha3-256 lorem" ) {
		REQUIRE(calculate_hash(lorem) == lorem_hash_256);
	}

	SECTION( "sha3-256 empty chunked" ) {
		REQUIRE(calculate_hash_multi(empty) == empty_hash_256);
	}
	SECTION( "sha3-256 abc chunk" ) {
		REQUIRE(calculate_hash_multi(abc) == abc_hash_256);
	}
	SECTION( "sha3-256 hello chunk" ) {
		REQUIRE(calculate_hash_multi(hello) == hello_hash_256);
	}
	SECTION( "sha3-256 lorem chunk" ) {
		REQUIRE(calculate_hash_multi(lorem) == lorem_hash_256);
	}
}

TEST_CASE( "rsa encryption and decryption" ) {
	SECTION( "rsa key generation" ) {
		// @note This test take a long time and we can't really verify anything
		// auto keys = crypto::generate_rsa_keys(1024);
        //
		// crypto::store("test/key-temp.priv", keys.first);
		// crypto::store("test/key-temp.pub", keys.second);
		// @todo Test if files exists and remove them again
	}

	SECTION( "rsa decrypt short") {
		auto priv = crypto::load("test/key.priv");
		auto encrypted = priv.encrypt(rsa_short);

		REQUIRE(bytes_to_string(encrypted) == rsa_short_encrypted);
	}
}
