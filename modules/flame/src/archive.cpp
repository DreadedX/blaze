#include <iostream>

#include "archive.h"
#include "async_fstream.h"
#include "async_data.h"
#include "asset.h"
#include "asset_list.h"
#include "tasks.h"

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

	Archive::Archive(std::shared_ptr<ASyncFStream> afs) : _afs(afs) {
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
						binary::read(fs, _name);
						binary::read(fs, _author);
						binary::read(fs, _description);
						binary::read(fs, _version);

						while (true) {
							uint8_t next;
							binary::read(fs, next);
							if (next == 0x00) {
								 break;
							}
							fs.seekg(-1, std::ios::cur);

							std::string name;
							binary::read(fs, name);
							uint16_t version;
							binary::read(fs, version);
							_dependencies.push_back(std::pair<std::string, uint16_t>(name, version));
						}

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

	Archive::Archive(std::shared_ptr<ASyncFStream> afs, std::string name, std::string author, std::string description, uint16_t version) : _afs(afs), _name(name), _author(author), _description(description), _version(version) {}

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

			binary::write(fs, _name);
			binary::write(fs, _author);
			binary::write(fs, _description);
			binary::write(fs, _version);

			for (auto& dependency : _dependencies) {
				binary::write(fs, dependency.first);
				binary::write(fs, dependency.second);
			}
			binary::write(fs, (uint8_t) 0x00);

			_afs->unlock();

			_initialized = true;
		} else {
			std::cerr << __FILE__ << ":" << __LINE__ << " " << "File stream closed\n";
		}
	}

	void Archive::finialize(std::array<uint8_t, 1217>& priv_key) {
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
			privqueue.Put2(priv_key.data(), priv_key.size(), 0, true);
			rsa_private.Load(privqueue);

			CryptoPP::RSA::PublicKey rsa_public(rsa_private);
			rsa_public.Save(pubqueue);
		}

		// Sign with private key
		CryptoPP::Integer signature = rsa_private.CalculateInverse(rnd, CryptoPP::Integer(digest.get(), HASH_SIZE));

		if (_afs && _afs->is_open()) {
			auto& fs = _afs->lock();
			fs.seekp(sizeof(MAGIC));
			binary::write(fs, pubqueue);
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

	void Archive::add(Asset& asset) {
		if (_valid) {
			std::cerr << __FILE__ << ":" << __LINE__ << " " << "Archive is already finalized\n";
			return;
		}
		if (!_initialized) {
			std::cerr << __FILE__ << ":" << __LINE__ << " " << "You need to initialze the archive first\n";
			return;
		}

		// @todo This is kind of janky.
		// @todo Also add compression here
		auto& old_workflow = asset.get_workflow();
		Asset::Workflow new_workflow = old_workflow;
		new_workflow.inner.push_back(add_chunk_marker);
		asset.set_workflow(new_workflow);

		auto data = asset.get_data();

		asset.set_workflow(old_workflow);

		if (_afs && _afs->is_open()) {
			auto& fs = _afs->lock();

			binary::write(fs, asset.get_name());
			binary::write(fs, asset.get_version());
			// We save the size of the data stream, not the size of the file on disk
			// @note This blocks until the stream is finished loading
			binary::write(fs, data.get_size());

			binary::write(fs, data.data(), data.get_size());

			_afs->unlock();
		} else {
			std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "File stream closed\n";
		}
	}

	void Archive::add_dependency(std::string name, uint16_t version) {
		if (_initialized) {
			std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "Can not add dependecies after initializing\n";
			return;
		}
		_dependencies.push_back(std::pair<std::string, uint16_t>(name, version));
	}

	const bool& Archive::is_valid() const { return _valid; }
	bool Archive::is_trusted(uint8_t trusted_key[]) { return binary::compare(_key, trusted_key, PUBLIC_KEY_SIZE); };

	const std::string& Archive::get_name() const {
		return _name;
	}

	const std::string& Archive::get_author() const {
		return _author;
	}

	const std::string& Archive::get_description() const {
		return _description;
	}

	const uint16_t& Archive::get_version() const {
		return _version;
	}

	const std::vector<std::pair<std::string, uint16_t>>& Archive::get_dependencies() const {
		return _dependencies;
	}

	// @todo Should we precompute this, or just compute it as we need it
	std::vector<Asset> Archive::get_assets() {
		std::vector<Asset> assets;

		if (!_valid) {
			std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "Archive invalid\n";
			return assets;
		}

		// Use the file stream to load all individual assets into Assets and add them to the list
		auto& fs = _afs->lock();

		fs.seekg(0, std::ios::end);
		unsigned long archive_size = fs.tellg();

		unsigned long next_asset = PUBLIC_KEY_SIZE + SIGNATURE_SIZE + sizeof(MAGIC) + _name.length()+1 + _author.length()+1 +_description.length()+1 + sizeof(uint16_t) /* version */;

		for (auto& dependency : _dependencies) {
			next_asset += dependency.first.length()+1 + sizeof(uint16_t);
		}
		// null terminator at the end of the dependency list
		next_asset += 1;

		while (next_asset < archive_size) {
			fs.seekg(next_asset);
			std::string name;
			binary::read(fs, name);
			uint16_t version;
			binary::read(fs, version);
			uint32_t size;
			binary::read(fs, size);
			uint32_t offset = fs.tellg();

			next_asset = offset + size;

			// @todo We are assuming that all files in the archive use chunk markers, but that might not be the case
			assets.push_back(Asset(name, _afs, version, offset, size, true));
		}

		_afs->unlock();

		return assets;
	}
}
