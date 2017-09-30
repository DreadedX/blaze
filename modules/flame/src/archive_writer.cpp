#include "archive_writer.h"
#include "file_handler.h"
#include "asset_data.h"
#include "binary_helper.h"
#include "meta_asset.h"
#include "tasks.h"

#include "rsa.h"
#include "osrng.h"

namespace FLAME_NAMESPACE {

	ArchiveWriter::ArchiveWriter(std::shared_ptr<FileHandler> fh, std::string name, std::string author, std::string description, uint16_t version) : _fh(fh), _name(name), _author(author), _description(description), _version(version) {}

	void ArchiveWriter::initialize() {
		if (_initialized) {
			throw std::logic_error("Archive is already initialized");
		}

		if (_fh && _fh->is_open()) {
			// Magic number
			auto& fs = _fh->lock();
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

			_fh->unlock();

			_initialized = true;
		} else {
			throw std::runtime_error("File stream closed");
		}
	}

	void ArchiveWriter::finalize(std::array<uint8_t, 1217>& priv_key) {
		if (_valid) {
			throw std::logic_error("Archive is already finalized");
		}
		if (!_initialized) {
			throw std::logic_error("You need to initialize the archive first");
		}

		// Calculate hash
		std::unique_ptr<uint8_t[]> digest;
		if (_fh && _fh->is_open()) {
			auto& fs = _fh->lock();
			fs.seekp(0, std::ios::end);
			auto size = fs.tellp();
			_fh->unlock();
			digest = calculate_hash(_fh, size);
		} else {
			throw std::runtime_error("File stream closed");
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

		if (_fh && _fh->is_open()) {
			auto& fs = _fh->lock();
			fs.seekp(sizeof(MAGIC));
			binary::write(fs, pubqueue);
			binary::write(fs, signature);

			_fh->unlock();

			// Store the public key
			assert(pubqueue.CurrentSize() == PUBLIC_KEY_SIZE);

			_valid = true;
		} else {
			throw std::runtime_error("File stream closed");
		}
	}

	void ArchiveWriter::add(MetaAsset& meta_asset) {
		if (_valid) {
			throw std::logic_error("Archive is already finalized");
		}
		if (!_initialized) {
			throw std::logic_error("You need to initialize the archive first");
		}

		MetaAsset::Workflow workflow;
		workflow.tasks.push_back(zlib::compress);

		auto data = meta_asset.get_data(workflow);

		if (_fh && _fh->is_open()) {
			auto& fs = _fh->lock();

			binary::write(fs, meta_asset.get_name());
			binary::write(fs, meta_asset.get_version());
			// We save the size of the data stream, not the size of the file on disk
			// @note This blocks until the stream is finished loading
			binary::write(fs, data.get_size());

			binary::write(fs, data.data(), data.get_size());

			_fh->unlock();
		} else {
			throw std::runtime_error("File stream closed");
		}
	}

	void ArchiveWriter::add_dependency(std::string name, uint16_t version) {
		if (_initialized) {
			throw std::logic_error("You cannot add dependecies after initializing");
		}
		_dependencies.push_back(std::pair<std::string, uint16_t>(name, version));
	}
}
