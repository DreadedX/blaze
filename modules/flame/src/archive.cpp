#include <iostream>

#include "archive.h"
#include "asset_data.h"
#include "meta_asset.h"
#include "asset_list.h"
#include "tasks.h"
#include "binary_helper.h"

#include "rsa.h"
#include "osrng.h"

#define CHUNK_SIZE 1024

namespace FLAME_NAMESPACE {

	std::unique_ptr<uint8_t[]> calculate_hash(std::shared_ptr<FileHandler> fh, uint32_t size) {
		auto& fs = fh->lock();

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

		fh->unlock();

		return digest;
	}

	Archive::Archive(std::shared_ptr<FileHandler> fh) : _fh(fh) {
		if (_fh && _fh->is_open()) {
			auto& fs = _fh->lock();
			fs.seekg(0, std::ios::end);
			unsigned long size = fs.tellg();
			fs.seekg(0);
			// If the file is smaller than this it is definetly not valid
			if (size >= SIGNATURE_SIZE + PUBLIC_KEY_SIZE + sizeof(MAGIC)) {
				uint8_t magic[sizeof(MAGIC)];
				binary::read(fs, magic, sizeof(MAGIC));
				if (binary::compare(magic, reinterpret_cast<const uint8_t*>(MAGIC), sizeof(MAGIC))) {
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
					// @todo length is sometimes 49 instead of 32
					std::cout << "Length: " << length << ", HASH_SIZE: " << HASH_SIZE << '\n';
					assert(length == HASH_SIZE);
					uint8_t stored_digest[length];
					stored_digest_integer.Encode(stored_digest, length);

					_fh->unlock();
					std::unique_ptr<uint8_t[]> digest = calculate_hash(_fh, size);
					_fh->lock();

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
					} else {
						_fh->unlock();
						throw std::runtime_error("File is corrupted or not finalized");
					}
				} else {
					_fh->unlock();
					throw std::runtime_error("File is not a FLMb file");
				}
			} else {
				_fh->unlock();
				throw std::runtime_error("File too small");
			}
			_fh->unlock();
		} else {
			throw std::runtime_error("File stream closed");
		}
	}

	const std::string& Archive::get_name() const {
		return _name;
	}

	const std::string& Archive::get_author() const {
		return _author;
	}

	const std::string& Archive::get_description() const {
		return _description;
	}

	bool Archive::is_trusted(uint8_t trusted_key[]) {
		return binary::compare(_key, trusted_key, PUBLIC_KEY_SIZE);
	}

	const uint16_t& Archive::get_version() const {
		return _version;
	}

	const std::vector<std::pair<std::string, uint16_t>>& Archive::get_dependencies() const {
		return _dependencies;
	}

	// @todo Should we precompute this, or just compute it as we need it
	std::vector<MetaAsset> Archive::get_meta_assets() {
		std::vector<MetaAsset> meta_assets;

		// Use the file stream to load all individual assets into Assets and add them to the list
		auto& fs = _fh->lock();

		fs.seekg(0, std::ios::end);
		unsigned long archive_size = fs.tellg();

		unsigned long next_meta_asset = PUBLIC_KEY_SIZE + SIGNATURE_SIZE + sizeof(MAGIC) + _name.length()+1 + _author.length()+1 +_description.length()+1 + sizeof(uint16_t) /* version */;

		for (auto& dependency : _dependencies) {
			next_meta_asset += dependency.first.length()+1 + sizeof(uint16_t);
		}
		// null terminator at the end of the dependency list
		next_meta_asset += 1;

		MetaAsset::Workflow workflow;
		workflow.tasks.push_back(zlib::decompress);

		while (next_meta_asset < archive_size) {
			fs.seekg(next_meta_asset);
			std::string name;
			binary::read(fs, name);
			uint16_t version;
			binary::read(fs, version);
			uint32_t size;
			binary::read(fs, size);
			uint32_t offset = fs.tellg();

			next_meta_asset = offset + size;

			meta_assets.push_back(MetaAsset(name, _fh, version, offset, size, workflow));
		}

		_fh->unlock();

		return meta_assets;
	}
}
