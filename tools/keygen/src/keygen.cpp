#include <fstream>

#include "binary_helper.h"
#include "archive.h"

// @todo We need this because of name conflict with CryptoPP, until we implement RSA ourselves aswell
#include "/home/tim/Projects/cpp/blaze/modules/crypto/include/rsa.h"

int main() {
	// CryptoPP::AutoSeededRandomPool rnd;
    //
	// CryptoPP::RSA::PrivateKey rsa_private;
	// rsa_private.GenerateRandomWithKeySize(rnd, FLAME_NAMESPACE::PRIVATE_KEY_BIT_SIZE);
    //
	// CryptoPP::RSA::PublicKey rsa_public(rsa_private);
    //
	// // @todo We need to make these not hardcoded
	// std::fstream pubfile("../keys/test.pub", std::ios::out);
	// if (!pubfile.is_open()) {
	// 	throw std::runtime_error("Failed to open keys/test.pub");
	// }
	// std::fstream privfile("../keys/test.priv", std::ios::out);
	// if (!privfile.is_open()) {
	// 	throw std::runtime_error("Failed to open keys/test.priv");
	// }
	// CryptoPP::ByteQueue pubqueue;
	// CryptoPP::ByteQueue privqueue;
	// rsa_public.Save(pubqueue);
	// rsa_private.Save(privqueue);
	// FLAME_NAMESPACE::binary::write(pubfile, pubqueue);
	// FLAME_NAMESPACE::binary::write(privfile, privqueue);

	// @todo Command line arguments for picking the size and name
	auto keys = crypto::generate_rsa_keys(1024);
	// @note We do not have to store the public key as we always use e=65537
	crypto::store("../keys/test_2.priv", keys.first);

}
