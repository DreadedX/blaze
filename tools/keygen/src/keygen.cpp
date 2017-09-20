#include <fstream>

#include "binary_helper.h"
#include "archive.h"

#include "rsa.h"
#include "osrng.h"

int main() {
	CryptoPP::AutoSeededRandomPool rnd;

	CryptoPP::RSA::PrivateKey rsa_private;
	rsa_private.GenerateRandomWithKeySize(rnd, blaze::flame::PRIVATE_KEY_BIT_SIZE);

	CryptoPP::RSA::PublicKey rsa_public(rsa_private);

	std::fstream pubfile("keys/test.pub", std::ios::out);
	std::fstream privfile("keys/test.priv", std::ios::out);
	CryptoPP::ByteQueue pubqueue;
	CryptoPP::ByteQueue privqueue;
	rsa_public.Save(pubqueue);
	rsa_private.Save(privqueue);
	blaze::flame::binary::write(pubfile, pubqueue);
	blaze::flame::binary::write(privfile, privqueue);
}
