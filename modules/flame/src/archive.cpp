#include <iostream>

#include "archive.h"
#include "async_fstream.h"
#include "async_data.h"
#include "asset.h"
#include "binary_helper.h"
#include "asset_list.h"

#include "sha3.h"
#include "rsa.h"
#include "osrng.h"

#define CHUNK_SIZE 1024

namespace blaze::flame {

	std::unique_ptr<uint8_t[]> calculate_hash(std::shared_ptr<ASyncFStream> afs, uint32_t size) {
		auto& fs = afs->lock();

		size -= SIGNATURE_SIZE + PUBLIC_KEY_SIZE + sizeof(MAGIC); 
		fs.seekg(SIGNATURE_SIZE + PUBLIC_KEY_SIZE + sizeof(MAGIC));

		uint32_t remaining = size;
		HASH_ALOGRITHM hash;
		while (remaining > 0) {
			uint32_t chunk = remaining < CHUNK_SIZE ? remaining : CHUNK_SIZE;

			std::unique_ptr<uint8_t[]> data = std::make_unique<uint8_t[]>(chunk);
			fs.read(reinterpret_cast<char*>(data.get()), chunk);
			hash.Update(data.get(), chunk);

			remaining -= chunk;
		}

		std::unique_ptr<uint8_t[]> digest = std::make_unique<uint8_t[]>(HASH_SIZE);
		hash.Final(digest.get());

		afs->unlock();

		return digest;
	}

	Archive::Archive(std::shared_ptr<ASyncFStream> afs, std::string author, std::string description) : _afs(afs), _author(author), _description(description) {}

	Archive::Archive(std::shared_ptr<ASyncFStream> afs) : _afs(afs) {
		std::cout << "Loading existing archive\n";
		if (_afs && _afs->is_open()) {
			auto& fs = _afs->lock();
			fs.seekg(0, std::ios::end);
			unsigned long size = fs.tellg();
			fs.seekg(0);
			// If the file is smaller than this it is definetly not valid
			if (size >= SIGNATURE_SIZE + PUBLIC_KEY_SIZE + sizeof(MAGIC)) {
				uint8_t magic[sizeof(MAGIC)];
				binary::read(fs, magic, sizeof(MAGIC));
				if (binary::compare(magic, reinterpret_cast<const uint8_t*>(MAGIC), sizeof(MAGIC))) {

					// We know the archive is at least initialized
					_initialized = true;

					binary::read(fs, _key, PUBLIC_KEY_SIZE);
					CryptoPP::RSA::PublicKey rsa_public;
					{
						CryptoPP::ByteQueue pubqueue;
						pubqueue.Put2(_key, PUBLIC_KEY_SIZE, 0, true);
						rsa_public.Load(pubqueue);
					}

					std::unique_ptr<uint8_t[]> signature = std::make_unique<uint8_t[]>(SIGNATURE_SIZE);
					binary::read(fs, signature.get(), SIGNATURE_SIZE);
					CryptoPP::Integer stored_digest_integer = rsa_public.ApplyFunction(CryptoPP::Integer(signature.get(), SIGNATURE_SIZE));

					size_t length = stored_digest_integer.MinEncodedSize();
					assert(length == HASH_SIZE);
					uint8_t stored_digest[length];
					stored_digest_integer.Encode(stored_digest, length);

					_afs->unlock();
					std::unique_ptr<uint8_t[]> digest = calculate_hash(_afs, size);
					_afs->lock();

					fs.seekg(PUBLIC_KEY_SIZE + SIGNATURE_SIZE + sizeof(MAGIC));

					if (binary::compare(digest.get(), stored_digest, HASH_SIZE)) {
						binary::read(fs, _author);
						binary::read(fs, _description);
						_valid = true;
					} else {
						std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "File is corrupted or not finalized\n";
					}
				} else {
					std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "File is not a FLMb file\n";
				}
			} else {
				std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "File is to small\n";
			}
			_afs->unlock();
		} else {
			std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "File stream closed\n";
		}
	}

	void Archive::initialize() {
		if (_initialized) {
			std::cerr << __FILE__ << ":" << __LINE__ << " " << "Archive is already initialized\n";
			return;
		}

		if (_afs && _afs->is_open()) {
			// Magic number
			auto& fs = _afs->lock();
			binary::write(fs, MAGIC, sizeof(MAGIC));

			// Reserve space for Signature and Key
			for (size_t i = 0; i < SIGNATURE_SIZE + PUBLIC_KEY_SIZE; ++i) {
				binary::write(fs, (uint8_t) 0x00);
			}

			binary::write(fs, _author);
			binary::write(fs, _description);

			_afs->unlock();

			_initialized = true;
		} else {
			std::cerr << __FILE__ << ":" << __LINE__ << " " << "File stream closed\n";
		}
	}

	void Archive::finialize(std::shared_ptr<uint8_t[]> priv_key, uint32_t key_size) {
		if (_valid) {
			std::cerr << __FILE__ << ":" << __LINE__ << " " << "Archive is already finalized";
			return;
		}
		if (!_initialized) {
			std::cerr << __FILE__ << ":" << __LINE__ << " " << "You need to initialze the archive first\n";
			return;
		}

		// Calculate hash
		std::unique_ptr<uint8_t[]> digest;
		if (_afs && _afs->is_open()) {
			auto& fs = _afs->lock();
			fs.seekp(0, std::ios::end);
			auto size = fs.tellp();
			_afs->unlock();
			digest = calculate_hash(_afs, size);
		} else {
			std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "File stream closed\n";
			return;
		}

		// Load keys
		CryptoPP::AutoSeededRandomPool rnd;
		CryptoPP::ByteQueue pubqueue;
		CryptoPP::RSA::PrivateKey rsa_private;

		{
			CryptoPP::ByteQueue privqueue;
			privqueue.Put2(priv_key.get(), key_size, 0, true);
			rsa_private.Load(privqueue);

			CryptoPP::RSA::PublicKey rsa_public(rsa_private);
			rsa_public.Save(pubqueue);
		}

		// Sign with private key
		CryptoPP::Integer signature = rsa_private.CalculateInverse(rnd, CryptoPP::Integer(digest.get(), HASH_SIZE));

		std::cout << "Signature: " << std::hex << signature << '\n';
		std::cout << "Hash: ";
		for (int i = 0; i < 32; ++i) {
			std::cout << "0x" << std::hex << (uint32_t)digest[i] << ' ';
		}
		std::cout << '\n';

		if (_afs && _afs->is_open()) {
			// Store current location in the file
			auto& fs = _afs->lock();
			// Go to hash portion of the archive
			fs.seekp(sizeof(MAGIC));
			// Write the public key
			binary::write(fs, pubqueue);
			// Write the (signed) hash
			binary::write(fs, signature);

			_afs->unlock();

			// Store the public key
			assert(pubqueue.CurrentSize() == PUBLIC_KEY_SIZE);
			for (int i = 0; i < PUBLIC_KEY_SIZE; ++i) {
				_key[i] = pubqueue[i];
			}

			_valid = true;
		} else {
			std::cerr << __FILE__ << ":" << __LINE__ << " " << "File stream closed\n";
		}
	}

	Archive& operator<<(Archive& archive, Asset& asset) {
		if (archive._valid) {
			std::cerr << __FILE__ << ":" << __LINE__ << " " << "Archive is already finalized";
			return archive;
		}
		if (!archive._initialized) {
			std::cerr << __FILE__ << ":" << __LINE__ << " " << "You need to initialze the archive first\n";
			return archive;
		}

		auto data = asset.get_data();

		if (archive._afs && archive._afs->is_open()) {
			auto& fs = archive._afs->lock();

			binary::write(fs, asset._name);
			binary::write(fs, asset._version);
			// We save the size of the data stream, not the size of the file on disk
			// @note This blocks until the stream is finished loading
			binary::write(fs, data.get_size());

			binary::write(fs, data.data(), data.get_size());

			archive._afs->unlock();
		} else {
			std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "File stream closed\n";
		}
		return archive;
	}

	// @todo Check if the archive is a valid one
	AssetList& operator<<(AssetList& asset_list, Archive& archive) {
		// Use the file stream to load all individual assets into Assets and add them to the list
		auto afs = archive.get_async_fstream();
		auto& fs = afs->lock();

		fs.seekg(0, std::ios::end);
		unsigned long archive_size = fs.tellg();

		unsigned long next_asset = PUBLIC_KEY_SIZE + SIGNATURE_SIZE + sizeof(MAGIC) + archive.get_author().length()+1 +archive.get_description().length()+1;

		while (next_asset < archive_size) {
			fs.seekg(next_asset);
			std::string name;
			binary::read(fs, name);
			uint8_t version;
			binary::read(fs, version);
			uint32_t size;
			binary::read(fs, size);
			uint32_t offset = fs.tellg();

			next_asset = offset + size;

			// Check if we have already 
			auto existing = asset_list._assets.find(name);
			if (existing != asset_list._assets.end()) {
				if (existing->second.get_version() < version) {
					std::cout << "Replacing asset with newer version: " << name << '\n';
				} else if(existing->second.get_version() > version) {
					std::cout << "Already loaded newer asset: " << name << '\n';
					continue;
				} else {
					std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "Conflicting assets with same version\n";
					// There is no way we can handle this situation, so we just really on load order
					continue;
				}
			}

			Asset asset(name, afs, version, offset, size);
			asset_list._assets[name] = asset;
		}

		afs->unlock();
		return asset_list;
	}
}
