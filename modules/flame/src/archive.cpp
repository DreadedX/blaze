#include <iostream>

#include "archive.h"
#include "async_fstream.h"
#include "asset.h"
#include "binary_helper.h"

#include "sha3.h"
#include "rsa.h"
#include "osrng.h"

namespace blaze::flame {
	void Archive::initialize() {
		if (_afs && _afs->is_open()) {
			// Magic number
			auto& fs = _afs->lock();
			binary::write(fs, (uint8_t) 'F');
			binary::write(fs, (uint8_t) 'L');
			binary::write(fs, (uint8_t) 'M');
			binary::write(fs, (uint8_t) 'a');

			// Reserve space for Signature and Key
			for (size_t i = 0; i < SIGNATURE_SIZE + PUBLIC_KEY_SIZE; ++i) {
				binary::write(fs, (uint8_t) 0);
			}

			binary::write(fs, _author);
			binary::write(fs, _description);

			_afs->unlock();
		} else {
			std::cerr << __FILE__ << ":" << __LINE__ << " " << "File stream closed\n";
		}
	}

	void Archive::finialize() {
		// Setup hash
		HASH_ALOGRITHM hash;
		uint8_t digest[HASH_SIZE];

		// Load data in hash
		// @todo Make this actually do something
		{
			std::string message = "This is a test";
			hash.Update((const uint8_t*)message.data(), message.length());
			hash.Final(digest);
		}

		CryptoPP::AutoSeededRandomPool rnd;

		CryptoPP::RSA::PrivateKey rsa_private;
		// @todo Do not hard code this in here
		std::fstream privfile("priv.key", std::ios::in);
		CryptoPP::ByteQueue privqueue;
		binary::read(privfile, privqueue, 1792);
		rsa_private.Load(privqueue);
		CryptoPP::RSA::PublicKey rsa_public(rsa_private);
		CryptoPP::ByteQueue pubqueue;
		rsa_public.Save(pubqueue);

		CryptoPP::Integer signed_hash = rsa_private.CalculateInverse(rnd, CryptoPP::Integer(digest, HASH_SIZE));

		std::cout << "Signed hash: " << std::hex << signed_hash << '\n';

		if (_afs && _afs->is_open()) {
			// Store current location in the file
			auto& fs = _afs->lock();
			auto pos = fs.tellp();
			// Go to hash portion of the archive
			fs.seekp(4);
			// Write the (signed) hash
			binary::write(fs, signed_hash);
			binary::write(fs, pubqueue);

			// Go back to where we left off
			fs.seekp(pos);
			_afs->unlock();
		} else {
			std::cerr << __FILE__ << ":" << __LINE__ << " " << "File stream closed\n";
		}
	}

	Archive& operator<<(Archive& archive, Asset& asset) {
		auto data = asset.get_data();

		if (archive._afs && archive._afs->is_open()) {
			auto& fs = archive._afs->lock();

			binary::write(fs, asset._name);
			binary::write(fs, asset._version);
			// We save the size of the data stream, not the size of the file on disk
			// @note This blocks until the stream is finished loading
			binary::write(fs, data->get_size());

			binary::write(fs, data->data(), data->get_size());

			archive._afs->unlock();

			return archive;
		} else {
			std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "File stream closed\n";
		}
	}
}
