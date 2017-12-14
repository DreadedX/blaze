#include <iostream>

#include "archive.h"
#include "asset_data.h"
#include "tasks.h"
#include "binary_helper.h"

#include "sha3.h"

#define CHUNK_SIZE 1024

namespace FLAME_NAMESPACE {

	std::vector<uint8_t> calculate_hash(std::shared_ptr<FileHandler> fh, uint32_t size) {
		if (!fh || !fh->is_open()) {
			throw std::runtime_error("File stream closed");
		}

		auto& fs = fh->lock();

		size -= 2*KEY_SIZE + sizeof(MAGIC); 
		fs.seekg(2*KEY_SIZE + sizeof(MAGIC));

		uint32_t remaining = size;
		crypto::SHA3_256 hash;
		while (remaining > 0) {
			uint32_t chunk = remaining < CHUNK_SIZE ? remaining : CHUNK_SIZE;

			std::vector<uint8_t> data(chunk);
			fs.read(reinterpret_cast<char*>(data.data()), chunk);
			hash.update(data);

			remaining -= chunk;
		}

		std::vector<uint8_t> digest = hash.finalize();

		assert(digest.size() == hash.digest_size());

		fh->unlock();

		return digest;
	}

	// @todo We need to make sure that each time we read we are staying withing file boundaries
	Archive::Archive(std::string filename) : _fh(std::make_shared<FileHandler>(filename, std::ios::in)), _key(std::vector<uint8_t>(), std::vector<uint8_t>()) {
		if (!_fh || !_fh->is_open()) {
			throw std::runtime_error("File stream closed");
		}

		auto& fs = _fh->lock();
		fs.seekg(0, std::ios::end);
		unsigned long size = fs.tellg();
		fs.seekg(0);
		// If the file is smaller than this it is definetly not valid
		// @todo Re-evaluate this
		if (size < 2*KEY_SIZE + sizeof(MAGIC)) {
			_fh->unlock();
			throw std::runtime_error("File too small");
		}

		uint8_t magic[sizeof(MAGIC)];
		binary::read(fs, magic, sizeof(MAGIC));
		if (!binary::compare(magic, MAGIC, sizeof(MAGIC))) {
			_fh->unlock();
			throw std::runtime_error("File is not a FLMb file");
		}

		// @todo There should be a function in crypto that provides this
		std::vector<uint8_t> n(KEY_SIZE);
		binary::read(fs, n.data(), KEY_SIZE);

		// @todo For some very weird reason this blocks if the key is corrupt and we have a lock (???)
		_fh->unlock();
		{
			// Make sure the key is not empty
			auto check = [n] {
				for (auto&& byte : n) {
					if (byte != 0x00) {
						return true;
					}
				}
				return false;
			};

			if (!check()) {
				throw std::runtime_error("File is corrupted");
			}
		}
		_key = crypto::RSA(n, crypto::default_e());
		if (!_fh || !_fh->is_open()) {
			throw std::runtime_error("File stream closed");
		}
		_fh->lock();

		std::vector<uint8_t> signature(KEY_SIZE);
		binary::read(fs, signature.data(), KEY_SIZE);
		// @todo Encrypt is the wrong term here
		std::vector<uint8_t> stored_digest = _key.encrypt(signature);

		_fh->unlock();

		std::vector<uint8_t> digest = calculate_hash(_fh, size);

		size_t length = stored_digest.size();
		if (length != digest.size()) {
			throw std::runtime_error("File is corrupted");
		}

		if (!binary::compare(digest.data(), stored_digest.data(), length)) {
			throw std::runtime_error("File is corrupted");
		}

		if (!_fh || !_fh->is_open()) {
			throw std::runtime_error("File stream closed");
		}
		_fh->lock();

		fs.seekg(2*KEY_SIZE + sizeof(MAGIC));
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


		unsigned long next_meta_asset = fs.tellg();
		while (next_meta_asset < size) {
			fs.seekg(next_meta_asset);
			std::string name;
			binary::read(fs, name);
			uint16_t version;
			binary::read(fs, version);
			uint8_t x;
			binary::read(fs, x);
			Compression compression = static_cast<Compression>(x);
			uint32_t size;
			binary::read(fs, size);
			uint32_t offset = fs.tellg();

			auto workflow = create_workflow(compression);

			next_meta_asset = offset + size;

			_meta_assets.push_back(MetaAsset(name, _fh, version, offset, size, workflow));
		}

		_fh->unlock();
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

	bool Archive::is_trusted(crypto::RSA& trusted_key) {
		// @todo Make vector compare, which checks if size if correct
		if (_key.get_d().size() != trusted_key.get_d().size()) {
			return false;
		}
		if (_key.get_n().size() != trusted_key.get_n().size()) {
			return false;
		}
		
		return binary::compare(_key.get_d().data(), trusted_key.get_d().data(), _key.get_d().size()) && binary::compare(_key.get_n().data(), trusted_key.get_n().data(), _key.get_n().size());
	}

	const uint16_t& Archive::get_version() const {
		return _version;
	}

	const std::vector<std::pair<std::string, uint16_t>>& Archive::get_dependencies() const {
		return _dependencies;
	}

	std::vector<MetaAsset::Task> Archive::create_workflow(Compression compression) {
		std::vector<MetaAsset::Task> workflow;

		switch (compression) {
			case Compression::none:
				break;
			case Compression::zlib:
				workflow.push_back(zlib::decompress);
				break;
			default:
				throw std::logic_error("Decompression type not implemented");
		}

		return workflow;
	}

	std::vector<MetaAsset> Archive::get_meta_assets() {
		return _meta_assets;
	}
}
