#include "/home/tim/Projects/cpp/blaze/modules/crypto/include/sha3.h"

#include <iostream>
#include <cstdint>

// int main() {
// 	crypto::SHA3_256 hash;
// 	for (size_t i = 0; i < 3; ++i) {
// 		std::vector<uint8_t> data;
// 		data.push_back(i+65);
// 		hash.update(data);
// 	}
//
// 	// std::vector<uint8_t> digest = crypto::sha3_256(data);
// 	auto digest = hash.finialize();
//
// 	for (auto b : digest) {
// 		std::cout << std::hex << (uint32_t)b;
// 	}
// 	std::cout << '\n';
// }
