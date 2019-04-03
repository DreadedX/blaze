#include "flame/archive.h"
#include "flame/data_handle.h"
#include "flame/tasks.h"

#include "iohelper/read.h"

#include "sha3.h"

#include <iostream>
#include <fstream>
#include <cassert>

#define CHUNK_SIZE 1024*1024

namespace FLAME_NAMESPACE {

	std::vector<uint8_t> calculate_hash(std::fstream& fs, size_t size, size_t offset) {
		fs.seekg(offset);

		uint32_t remaining = size;
		crypto::SHA3_256 hash;
		while (remaining > 0) {
			std::cout << "Hashing chunk\n";
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
	// @todo What is the purpose of the _key here
	Archive::Archive(std::string filename, Directory* root) : _key(std::vector<uint8_t>(), std::vector<uint8_t>()) {

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

		auto magic = iohelper::read_vector<uint8_t>(fs, MAGIC.size());
		if (!std::equal(magic.begin(), magic.begin(), MAGIC.end())) {
			throw std::runtime_error("File is not a FLMx file");
		}

		_signed = iohelper::read<bool>(fs);

		_offset1 = fs.tellg();

		std::vector<uint8_t> stored_digest;
		if (_signed) {
			// @todo There should be a function in crypto that provides this
			auto n = iohelper::read_vector<uint8_t>(fs);
			_key = crypto::RSA(n, crypto::default_e());

			auto signature = iohelper::read_vector<uint8_t>(fs);
			// @todo Encrypt is the wrong term here
			stored_digest = _key.encrypt(signature);
		} else {
			stored_digest = iohelper::read_vector<uint8_t>(fs);
		}

		_offset2 = fs.tellg();

		// @todo This is SUPER slow
		//std::vector<uint8_t> digest = calculate_hash(fs, size - _offset2, _offset2);

		//size_t length = stored_digest.size();
		//if (length != digest.size()) {
			//throw std::runtime_error("File is corrupted (Digest size is different)");
		//}

		//if (!std::equal(digest.begin(), digest.begin(), stored_digest.end())) {
			//throw std::runtime_error("File is corrupted (Digest is different)");
		//}

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

		unsigned long next_file_handle = fs.tellg();
		while (next_file_handle < size) {
			fs.seekg(next_file_handle);
			Type type = static_cast<Type>(iohelper::read<uint8_t>(fs));
			if (type == Type::dir) {
				int id = iohelper::read_length(fs);
				int parent = iohelper::read_length(fs);
				std::string name = iohelper::read<std::string>(fs);

				next_file_handle = fs.tellg();

				_folders.insert({id, {parent, name}});
			} else if(type == Type::file) {
				internal::FileInfo file_info = {};
				file_info.filename = filename;

				// Resolve the full asset name at runtime
				int parent = iohelper::read_length(fs);
				std::string name = iohelper::read<std::string>(fs);
				std::vector<std::string> directory_names;
				while (parent) {
					auto& [folder_parent, folder_name] = _folders.find(parent)->second;
					directory_names.push_back(folder_name);
					parent = folder_parent;
				}
				std::reverse(directory_names.begin(), directory_names.end());
				Directory* parent_directory = root;
				for (auto& directory_name : directory_names) {
					Directory* new_parent_directory = parent_directory->get_directory(directory_name);
					if (!new_parent_directory) {
						new_parent_directory = new Directory(directory_name, parent_directory);
						parent_directory->add_directory(new_parent_directory);
					}

					parent_directory = new_parent_directory;
				}

				size_t version = iohelper::read_length(fs);
				Compression compression = static_cast<Compression>(iohelper::read<uint8_t>(fs));
				file_info.size = iohelper::read_length(fs);
				file_info.offset = fs.tellg();

				auto workflow = create_workflow(compression);

				next_file_handle = file_info.offset + file_info.size;

				parent_directory->add_file(FileHandle(name, version, file_info, workflow));
			} else {
				throw std::logic_error("This should not happen");
			}
		}

		// Print all folders
		// for (auto& [id, folder] : _folders) {
		// 	std::cout << id << " - > " << folder.second << " (" << folder.first << ")\n";
		// }

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

		return std::equal(_key.get_d().begin(), _key.get_d().end(), trusted_key.get_d().begin()) && std::equal(_key.get_n().begin(), _key.get_n().end(), trusted_key.get_n().begin());
	}

	const size_t& Archive::get_version() const {
		return _version;
	}

	const std::vector<Dependency>& Archive::get_dependencies() const {
		return _dependencies;
	}

	std::vector<FileHandle::Task> Archive::create_workflow(Compression compression) {
		std::vector<FileHandle::Task> workflow;

		switch (compression) {
			case Compression::none:
				break;
			case Compression::zlib:
				workflow.push_back(zlib::decompress);
				break;
			default:
				std::cerr << "COMPRESSION " << (uint32_t)compression << '\n';
				throw std::logic_error("Decompression type not implemented");
		}

		return workflow;
	}
}
