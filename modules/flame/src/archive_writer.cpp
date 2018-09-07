#include "flame/archive_writer.h"
#include "flame/file_handler.h"
#include "flame/asset_data.h"
#include "flame/binary_helper.h"
#include "flame/tasks.h"

#include "iohelper/write.h"
#include "iohelper/read.h"

namespace FLAME_NAMESPACE {

	// @todo Allow unsigned archives, these just store the digest
	// @todo Update flame to use iohelper and retire binary_helper
	// @todo We should add a function that skips ahead the correct amount based on the specified type (More iohelper related)
	// @current
	ArchiveWriter::ArchiveWriter(std::string name, std::string filename, std::string author, std::string description, uint16_t version, std::vector<Dependency> dependencies, crypto::RSA priv) : _fh(std::make_shared<FileHandler>(filename, std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary)), _name(name), _author(author), _description(description), _version(version), _dependencies(dependencies), _priv(priv) {
		if (!_fh || !_fh->is_open()) {
			throw std::runtime_error("File stream closed");
		}

		// Magic number
		auto& fs = _fh->lock();
		binary::write(fs, MAGIC, sizeof(MAGIC));

		// Reserve space for Signature and Key
		iohelper::write(fs, _priv.get_n());
		// @todo Figure out the correct way to determine the length of the key
		std::vector<uint8_t> digest_reserve((_priv.get_d().size()/8)*8);
		iohelper::write(fs, digest_reserve);

		// binary::write(fs, static_cast<uint8_t>(_compression));
		binary::write(fs, _name);
		binary::write(fs, _author);
		binary::write(fs, _description);
		// @todo What is the point of this when each archive contains a version as well...
		binary::write(fs, _version);

		for (auto& dependency : _dependencies) {
			binary::write(fs, std::get<0>(dependency));
			binary::write(fs, std::get<1>(dependency));
			binary::write(fs, std::get<2>(dependency));
		}
		binary::write(fs, (uint8_t) 0x00);

		_fh->unlock();
	}

	void ArchiveWriter::sign() {

		if (_signed) {
			throw std::logic_error("Archive is already finalized");
		}

		if (!_fh || !_fh->is_open()) {
			throw std::runtime_error("File stream closed");
		}
		auto& fs = _fh->lock();

		// Calculate hash
		fs.seekp(0, std::ios::end);
		auto size = fs.tellp();
		_fh->unlock();

		// @todo Make sure this function starts in the right place
		std::vector<uint8_t> digest = calculate_hash(_fh, size);

		// Sign with private key
		std::vector<uint8_t> signature = _priv.encrypt(digest);
		std::cout << "SIGNATURE SIZE: " << signature.size() << '\n';

		if (!_fh || !_fh->is_open()) {
			throw std::runtime_error("File stream closed");
		}
		_fh->lock();

		fs.seekp(sizeof(MAGIC));
		iohelper::read<std::vector<uint8_t>>(fs);
		iohelper::write(fs, signature);

		_fh->unlock();

		_signed = true;
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
		if (_signed) {
			throw std::logic_error("Archive is already finalized");
		}

		// Start loading
		// @note We run in deferred mode because there is no point here in running async
		auto data = meta_asset.get_data(false, create_workflow(compression));

		if (!_fh || !_fh->is_open()) {
			throw std::runtime_error("File stream closed");
		}

		auto& fs = _fh->lock();
		binary::write(fs, meta_asset.get_name());
		binary::write(fs, meta_asset.get_version());
		binary::write(fs, static_cast<uint8_t>(compression));
		// We save the size of the data stream, not the size of the file on disk
		// @note This blocks until the stream is finished loading

		binary::write(fs, data.get_size());
		binary::write(fs, data.as<uint8_t*>(), data.get_size());

		_fh->unlock();
	}
}
