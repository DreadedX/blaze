#include <fstream>

#include "binary_helper.h"
#include "archive.h"

#include "rsa.h"
#include "osrng.h"

int main() {
	CryptoPP::AutoSeededRandomPool rnd;

	CryptoPP::RSA::PrivateKey rsa_private;
	rsa_private.GenerateRandomWithKeySize(rnd, FLAME_NAMESPACE::PRIVATE_KEY_BIT_SIZE);

	CryptoPP::RSA::PublicKey rsa_public(rsa_private);

	std::fstream pubfile("keys/test.pub", std::ios::out);
	std::fstream privfile("keys/test.priv", std::ios::out);
	CryptoPP::ByteQueue pubqueue;
	CryptoPP::ByteQueue privqueue;
	rsa_public.Save(pubqueue);
	rsa_private.Save(privqueue);
	FLAME_NAMESPACE::binary::write(pubfile, pubqueue);
	FLAME_NAMESPACE::binary::write(privfile, privqueue);
}
