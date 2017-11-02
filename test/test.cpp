#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "data.h"

#include "/home/tim/Projects/cpp/blaze/modules/crypto/include/sha3.h"
#include <sstream>

// @note We are assuming that these functions are correct

// All data in one go
std::string calculate_hash(std::vector<uint8_t> data) {
	crypto::SHA3_256 hash;
	hash.update(data);
	std::vector<uint8_t> digest = hash.finalize();

	std::stringstream ss;
	for (auto&& d : digest) {
		ss << std::setfill('0') << std::setw(2) << std::hex << (uint32_t)d;
	}

	return ss.str();
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
