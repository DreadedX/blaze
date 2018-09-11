#include "flame/archive.h"
#include "flame/asset_data.h"
#include "flame/tasks.h"

#include "iohelper/read.h"

#include "sha3.h"

#include <iostream>
#include <fstream>
#include <cassert>

#define CHUNK_SIZE 1024

// @todo Get rid of this
namespace binary {
	bool compare(const uint8_t array1[], const uint8_t array2[], const uint32_t size) {
		for (uint32_t i = 0; i < size; ++i) {
			if (array1[i] != array2[i]) {
				return false;
			}
		}
		return true;
	}
}

namespace FLAME_NAMESPACE {

	std::vector<uint8_t> calculate_hash(std::fstream& fs, size_t size, size_t offset) {
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

		return digest;
	}

	// @todo We need to make sure that each time we read we are staying withing file boundaries
	// @todo What is the purpose of the _priv here
	Archive::Archive(std::string filename) : _priv(std::vector<uint8_t>(), std::vector<uint8_t>()) {

		std::fstream fs(filename, std::ios::in | std::ios::binary);
		if (!fs.is_open()) {
			throw std::runtime_error("Failed to open file");
		}

		fs.seekg(0, std::ios::end);
		unsigned long size = fs.tellg();
		fs.seekg(0);
		// If the file is smaller than this it is definetly not valid
		// @todo Re-evaluate this
		// if (size < 2*KEY_SIZE + sizeof(MAGIC)) {
		// 	_fh->unlock();
		// 	throw std::runtime_error("File too small");
		// }

		auto magic = iohelper::read<std::vector<uint8_t>>(fs, MAGIC.size());
		if (!binary::compare(magic.data(), MAGIC.data(), MAGIC.size())) {
			throw std::runtime_error("File is not a FLMx file");
		}

		_signed = iohelper::read<bool>(fs);

		_offset1 = fs.tellg();

		std::vector<uint8_t> stored_digest;
		if (_signed) {
			// @todo There should be a function in crypto that provides this
			auto n = iohelper::read<std::vector<uint8_t>>(fs);
			_priv = crypto::RSA(n, crypto::default_e());

			auto signature = iohelper::read<std::vector<uint8_t>>(fs);
			// @todo Encrypt is the wrong term here
			stored_digest = _priv.encrypt(signature);
		} else {
			stored_digest = iohelper::read<std::vector<uint8_t>>(fs);
		}

		_offset2 = fs.tellg();

		std::vector<uint8_t> digest = calculate_hash(fs, size - _offset2, _offset2);

		size_t length = stored_digest.size();
		if (length != digest.size()) {
			throw std::runtime_error("File is corrupted (Digest size is different)");
		}

		if (!binary::compare(digest.data(), stored_digest.data(), length)) {
			throw std::runtime_error("File is corrupted (Digest is different)");
		}

		// Jump to the start of the data
		fs.seekg(_offset2);

		_name = iohelper::read<std::string>(fs);
		std::cout << "NAME: " << _name << '\n';
		_author = iohelper::read<std::string>(fs);
		_description = iohelper::read<std::string>(fs);
		_version = iohelper::read_length(fs);

		auto count = iohelper::read_length(fs);
		for (size_t i = 0; i < count; ++i) {
			auto name = iohelper::read<std::string>(fs);;
			auto version_min = iohelper::read_length(fs);;
			auto version_max = iohelper::read_length(fs);;

			_dependencies.push_back(Dependency(name, version_min, version_max));
		}

		unsigned long next_meta_asset = fs.tellg();
		while (next_meta_asset < size) {
			fs.seekg(next_meta_asset);
			std::string name = iohelper::read<std::string>(fs);
			// @todo Meta asset should use size_t instead of uint16_t
			size_t version = iohelper::read_length(fs);
			Compression compression = static_cast<Compression>(iohelper::read<uint8_t>(fs));
			size_t size = iohelper::read_length(fs);
			size_t offset = fs.tellg();

			auto workflow = create_workflow(compression);

			next_meta_asset = offset + size;

			_meta_assets.push_back(MetaAsset(name, filename, version, offset, size, workflow));
		}

		fs.close();
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
		if (_priv.get_d().size() != trusted_key.get_d().size()) {
			return false;
		}
		if (_priv.get_n().size() != trusted_key.get_n().size()) {
			return false;
		}
		
		return binary::compare(_priv.get_d().data(), trusted_key.get_d().data(), _priv.get_d().size()) && binary::compare(_priv.get_n().data(), trusted_key.get_n().data(), _priv.get_n().size());
	}

	const size_t& Archive::get_version() const {
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
