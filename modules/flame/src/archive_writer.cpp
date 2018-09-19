#include "flame/archive_writer.h"
#include "flame/asset_data.h"
#include "flame/tasks.h"

#include "iohelper/write.h"
#include "iohelper/read.h"

namespace FLAME_NAMESPACE {

	// @current
	// @todo We should add a function that skips ahead the correct amount based on the specified type (More iohelper related)
	ArchiveWriter::ArchiveWriter(std::string name, std::string filename, std::string author, std::string description, size_t version, std::vector<Dependency> dependencies, crypto::RSA priv) : Archive(name, author, description, version, dependencies, priv), _fs(filename, std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary) {
		if (!_fs.is_open()) {
			throw std::runtime_error("Failed to open file");
		}

		iohelper::write(_fs, MAGIC, false);

		_signed = _key.get_d().size() & _key.get_n().size();
		iohelper::write<bool>(_fs, _signed);

		_offset1 = _fs.tellp();

		size_t digest_size;
		// @todo Figure out the correct way to determine the length of the digest
		if (_signed) {
			// Reserve space for Signature and Key
			iohelper::write(_fs, _key.get_n());
			digest_size = (_key.get_d().size()/8)*8;
		} else {
			// Reserve space for sha3 digest
			digest_size = 256/8;
		}
		std::vector<uint8_t> digest_reserve(digest_size);
		iohelper::write(_fs, digest_reserve);

		_offset2 = _fs.tellp();

		iohelper::write<std::string>(_fs, _name);
		iohelper::write<std::string>(_fs, _author);
		iohelper::write<std::string>(_fs, _description);
		// @todo What is the point of this when each archive contains a version as well...
		iohelper::write_length(_fs, _version);

		// We can have max of 65536 dependencies
		iohelper::write_length(_fs, _dependencies.size());
		for (auto& dependency : _dependencies) {
			iohelper::write<std::string>(_fs, std::get<0>(dependency));
			iohelper::write_length(_fs, std::get<1>(dependency));
			iohelper::write_length(_fs, std::get<2>(dependency));
		}
	}

	void ArchiveWriter::finalize() {
		if (_finalized) {
			throw std::logic_error("Archive is already finalized");
		}

		// Calculate hash
		_fs.seekp(0, std::ios::end);
		size_t size = _fs.tellp();

		// @todo Make sure this function starts in the right place
		std::vector<uint8_t> digest = calculate_hash(_fs, size - _offset2, _offset2);

		if (_signed) {
			// Sign with private key
			std::vector<uint8_t> signature = _key.encrypt(digest);
			std::cout << "SIGNATURE SIZE: " << signature.size() << '\n';

			_fs.seekp(_offset1);
			iohelper::read<std::vector<uint8_t>>(_fs);
			iohelper::write(_fs, signature);
		} else {
			_fs.seekp(_offset1);
			iohelper::write(_fs, digest);
		}

		_finalized = true;
		_fs.close();
	}

	std::vector<MetaAsset::Task> ArchiveWriter::create_workflow(Compression compression) {
		std::vector<MetaAsset::Task> workflow;

		switch (compression) {
			case Compression::none:
				break;
			case Compression::zlib:
				workflow.push_back(zlib::compress);
				break;
			default:
				throw std::logic_error("Compression type not implemented");
		}

		return workflow;
	}

	void ArchiveWriter::add(MetaAsset& meta_asset, Compression compression = Compression::none) {
		if (_finalized) {
			throw std::logic_error("Archive is already finalized");
		}

		// Start loading
		// @note We run in deferred mode because there is no point here in running async
		auto data = meta_asset.get_data(false, create_workflow(compression));

		iohelper::write<std::string>(_fs, meta_asset.get_name());
		iohelper::write_length(_fs, meta_asset.get_version());
		iohelper::write<uint8_t>(_fs, static_cast<uint8_t>(compression));

		iohelper::write(_fs, data.as<std::vector<uint8_t>&>());
	}
}
