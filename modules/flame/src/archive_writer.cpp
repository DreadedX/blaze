#include "flame/archive_writer.h"
#include "flame/file_handler.h"
#include "flame/asset_data.h"
#include "flame/binary_helper.h"
#include "flame/tasks.h"

#include "iohelper/write.h"
#include "iohelper/read.h"

namespace FLAME_NAMESPACE {

	// @todo Update flame to use iohelper and retire binary_helper
	// @todo We should add a function that skips ahead the correct amount based on the specified type (More iohelper related)
	// @current
	ArchiveWriter::ArchiveWriter(std::string name, std::string filename, std::string author, std::string description, size_t version, std::vector<Dependency> dependencies, crypto::RSA priv) : _fh(std::make_shared<FileHandler>(filename, std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary)), _name(name), _author(author), _description(description), _version(version), _dependencies(dependencies), _priv(priv) {
		if (!_fh || !_fh->is_open()) {
			throw std::runtime_error("File stream closed");
		}

		// Magic number
		auto& fs = _fh->lock();
		iohelper::write(fs, MAGIC, false);

		_signed = _priv.get_d().size() & _priv.get_n().size();
		iohelper::write<bool>(fs, _signed);

		_offset1 = fs.tellp();

		size_t digest_size;
		if (_signed) {
			// Reserve space for Signature and Key
			iohelper::write(fs, _priv.get_n());
			// @todo Figure out the correct way to determine the length of the key
			digest_size = (_priv.get_d().size()/8)*8;
		} else {
			// Reserve space for sha3 digest
			digest_size = 256/8;
		}
		std::vector<uint8_t> digest_reserve(digest_size);
		iohelper::write(fs, digest_reserve);

		_offset2 = fs.tellp();

		iohelper::write<std::string>(fs, _name);
		iohelper::write<std::string>(fs, _author);
		iohelper::write<std::string>(fs, _description);
		// @todo What is the point of this when each archive contains a version as well...
		iohelper::write_length(fs, _version);

		// We can have max of 65536 dependencies
		iohelper::write_length(fs, _dependencies.size());
		for (auto& dependency : _dependencies) {
			iohelper::write<std::string>(fs, std::get<0>(dependency));
			iohelper::write_length(fs, std::get<1>(dependency));
			iohelper::write_length(fs, std::get<2>(dependency));
		}

		_fh->unlock();
	}

	void ArchiveWriter::finalize() {
		if (_finalized) {
			throw std::logic_error("Archive is already finalized");
		}

		if (!_fh || !_fh->is_open()) {
			throw std::runtime_error("File stream closed");
		}
		auto& fs = _fh->lock();

		// Calculate hash
		fs.seekp(0, std::ios::end);
		size_t size = fs.tellp();
		_fh->unlock();

		// @todo Make sure this function starts in the right place
		std::vector<uint8_t> digest = calculate_hash(_fh, size - _offset2, _offset2);

		if (_signed) {
			// Sign with private key
			std::vector<uint8_t> signature = _priv.encrypt(digest);
			std::cout << "SIGNATURE SIZE: " << signature.size() << '\n';

			if (!_fh || !_fh->is_open()) {
				throw std::runtime_error("File stream closed");
			}
			_fh->lock();

			fs.seekp(_offset1);
			iohelper::read<std::vector<uint8_t>>(fs);
			iohelper::write(fs, signature);

			_fh->unlock();
		} else {
			if (!_fh || !_fh->is_open()) {
				throw std::runtime_error("File stream closed");
			}
			_fh->lock();

			fs.seekp(_offset1);
			iohelper::write(fs, digest);

			_fh->unlock();
		}

		_finalized = true;
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

		if (!_fh || !_fh->is_open()) {
			throw std::runtime_error("File stream closed");
		}

		auto& fs = _fh->lock();
		iohelper::write<std::string>(fs, meta_asset.get_name());
		iohelper::write_length(fs, meta_asset.get_version());
		iohelper::write<uint8_t>(fs, static_cast<uint8_t>(compression));

		iohelper::write_length(fs, data.get_size());
		// @todo We will keep this until we revampt the meta asset and asset data and file handler stuff 
		binary::write(fs, data.as<uint8_t*>(), data.get_size());

		_fh->unlock();
	}
}
