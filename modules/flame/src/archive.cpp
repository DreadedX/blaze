#include <iostream>

#include "flame/archive.h"
#include "flame/asset_data.h"
#include "flame/tasks.h"
#include "flame/binary_helper.h"

#include "iohelper/read.h"

#include "sha3.h"

#define CHUNK_SIZE 1024

namespace FLAME_NAMESPACE {

	std::vector<uint8_t> calculate_hash(std::shared_ptr<FileHandler> fh, size_t size, size_t offset) {
		if (!fh || !fh->is_open()) {
			throw std::runtime_error("File stream closed");
		}

		auto& fs = fh->lock();

		fs.seekg(offset);

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
	// @todo What is the purpose of the _key here
	Archive::Archive(std::string filename) : _fh(std::make_shared<FileHandler>(filename, std::ios::in | std::ios::binary)), _key(std::vector<uint8_t>(), std::vector<uint8_t>()) {
		if (!_fh || !_fh->is_open()) {
			throw std::runtime_error("File stream closed");
		}

		auto& fs = _fh->lock();
		fs.seekg(0, std::ios::end);
		unsigned long size = fs.tellg();
		fs.seekg(0);
		// If the file is smaller than this it is definetly not valid
		// @todo Re-evaluate this
		// if (size < 2*KEY_SIZE + sizeof(MAGIC)) {
		// 	_fh->unlock();
		// 	throw std::runtime_error("File too small");
		// }

		uint8_t magic[sizeof(MAGIC)];
		binary::read(fs, magic, sizeof(MAGIC));
		if (!binary::compare(magic, MAGIC, sizeof(MAGIC))) {
			_fh->unlock();
			throw std::runtime_error("File is not a FLMb file");
		}

		_signed = iohelper::read<bool>(fs);

		_offset1 = fs.tellg();

		std::vector<uint8_t> stored_digest;
		if (_signed) {
			// @todo There should be a function in crypto that provides this
			auto n = iohelper::read<std::vector<uint8_t>>(fs);
			_key = crypto::RSA(n, crypto::default_e());

			auto signature = iohelper::read<std::vector<uint8_t>>(fs);
			// @todo Encrypt is the wrong term here
			stored_digest = _key.encrypt(signature);
		} else {
			stored_digest = iohelper::read<std::vector<uint8_t>>(fs);
		}

		_offset2 = fs.tellg();

		_fh->unlock();

		std::vector<uint8_t> digest = calculate_hash(_fh, size - _offset2, _offset2);

		size_t length = stored_digest.size();
		if (length != digest.size()) {
			throw std::runtime_error("File is corrupted (Digest size is different)");
		}

		if (!binary::compare(digest.data(), stored_digest.data(), length)) {
			throw std::runtime_error("File is corrupted (Digest is different)");
		}

		if (!_fh || !_fh->is_open()) {
			throw std::runtime_error("File stream closed");
		}
		_fh->lock();

		// Jump to the start of the data
		fs.seekg(_offset2);

		_name = iohelper::read<std::string>(fs);
		_author = iohelper::read<std::string>(fs);
		_description = iohelper::read<std::string>(fs);
		_version = iohelper::read<uint16_t>(fs);

		uint16_t count = iohelper::read<uint16_t>(fs);
		for (int i = 0; i < count; ++i) {
			auto name = iohelper::read<std::string>(fs);;
			auto version_min = iohelper::read<uint16_t>(fs);;
			auto version_max = iohelper::read<uint16_t>(fs);;

			_dependencies.push_back(Dependency(name, version_min, version_max));
		}


		unsigned long next_meta_asset = fs.tellg();
		while (next_meta_asset < size) {
			fs.seekg(next_meta_asset);
			std::string name = iohelper::read<std::string>(fs);
			uint16_t version = iohelper::read<uint16_t>(fs);
			Compression compression = static_cast<Compression>(iohelper::read<uint8_t>(fs));
			uint32_t size = iohelper::read<uint32_t>(fs);
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
		if (!_signed) {
			return false;
		}

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

	const std::vector<Dependency>& Archive::get_dependencies() const {
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
