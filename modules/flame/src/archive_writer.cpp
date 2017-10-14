#include "archive_writer.h"
#include "file_handler.h"
#include "asset_data.h"
#include "binary_helper.h"
#include "tasks.h"

#include "rsa.h"
#include "osrng.h"

namespace FLAME_NAMESPACE {

	ArchiveWriter::ArchiveWriter(std::string name, std::shared_ptr<FileHandler> fh, std::string author, std::string description, uint16_t version, flame::Compression compression) : _fh(fh), _name(name), _author(author), _description(description), _version(version), _compression(compression) {}

	// @todo This needs to go in the constructor
	void ArchiveWriter::initialize() {
		if (_initialized) {
			throw std::logic_error("Archive is already initialized");
		}

		if (!_fh || !_fh->is_open()) {
			throw std::runtime_error("File stream closed");
		}

		// Magic number
		auto& fs = _fh->lock();
		binary::write(fs, MAGIC, sizeof(MAGIC));

		// Reserve space for Signature and Key
		for (size_t i = 0; i < SIGNATURE_SIZE + PUBLIC_KEY_SIZE; ++i) {
			binary::write(fs, (uint8_t) 0x00);
		}

		binary::write(fs, static_cast<uint8_t>(_compression));
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
	}

	void ArchiveWriter::finalize(std::array<uint8_t, 1217>& priv_key) {
		if (_valid) {
			throw std::logic_error("Archive is already finalized");
		}
		if (!_initialized) {
			throw std::logic_error("You need to initialize the archive first");
		}

		if (!_fh || !_fh->is_open()) {
			throw std::runtime_error("File stream closed");
		}
		auto& fs = _fh->lock();

		// Calculate hash
		fs.seekp(0, std::ios::end);
		auto size = fs.tellp();
		_fh->unlock();

		std::vector<uint8_t> digest = calculate_hash(_fh, size);

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
		CryptoPP::Integer signature = rsa_private.CalculateInverse(rnd, CryptoPP::Integer(digest.data(), HASH_SIZE));

		if (!_fh || !_fh->is_open()) {
			throw std::runtime_error("File stream closed");
		}
		_fh->lock();

		fs.seekp(sizeof(MAGIC));
		binary::write(fs, pubqueue);
		binary::write(fs, signature);

		_fh->unlock();

		// Store the public key
		assert(pubqueue.CurrentSize() == PUBLIC_KEY_SIZE);

		_valid = true;
	}

	MetaAsset::Workflow ArchiveWriter::create_workflow() {
		MetaAsset::Workflow workflow;

		switch (_compression) {
			case Compression::none:
				break;
			case Compression::zlib:
				workflow.tasks.push_back(zlib::compress);
				break;
			default:
				throw std::logic_error("Compression type not implemented");
		}

		return workflow;
	}

	void ArchiveWriter::add(MetaAsset& meta_asset) {
		if (_valid) {
			throw std::logic_error("Archive is already finalized");
		}
		if (!_initialized) {
			throw std::logic_error("You need to initialize the archive first");
		}

		// Start loading
		auto data = meta_asset.get_data(create_workflow());

		if (!_fh || !_fh->is_open()) {
			throw std::runtime_error("File stream closed");
		}

		auto& fs = _fh->lock();
		binary::write(fs, meta_asset.get_name());
		binary::write(fs, meta_asset.get_version());
		// We save the size of the data stream, not the size of the file on disk
		// @note This blocks until the stream is finished loading

		binary::write(fs, data.get_size());
		binary::write(fs, data.data(), data.get_size());

		_fh->unlock();
	}

	// @todo This should just be set in the constructor
	void ArchiveWriter::add_dependency(std::string name, uint16_t version) {
		if (_initialized) {
			throw std::logic_error("You cannot add dependecies after initializing");
		}
		_dependencies.push_back(std::pair<std::string, uint16_t>(name, version));
	}
}
