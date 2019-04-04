#pragma once

#include "flame.h"
// #include "flame/file_handle.h"
#include "flame/vfs.h"

#include "rsa.h"

namespace FLAME_NAMESPACE {

	enum class Compression : uint8_t {
		none,
		zlib
	};

	enum class Type : uint8_t {
		file,
		dir
	};

	typedef std::tuple<std::string, size_t, size_t> Dependency;

	// FLMx is the new wip spec
	const std::vector<uint8_t> MAGIC = {'F','L','M','x'};
	// 1024 bit key
	const int KEY_SIZE = 1024/8;

	std::vector<uint8_t> calculate_hash(std::fstream& fs, size_t size, size_t offset = 0);

	// Archives only exist for writing files
	class Archive {
		public:
			Archive(std::string filename);

			const std::string& get_name() const;
			const std::string& get_author() const;
			const std::string& get_description() const;
			const size_t& get_version() const;
			bool is_trusted(crypto::RSA& trusted_key);
			const std::vector<Dependency>& get_dependencies() const;
			// std::vector<FileHandle> get_file_handles();
			Directory* get_directory() {
				return _directory;
			}

		protected:
			Archive(std::string name, std::string author, std::string description, size_t version, std::vector<Dependency> dependencies, crypto::RSA key) : _name(name), _author(author), _description(description), _version(version), _dependencies(dependencies), _key(key) {}

			std::vector<FileHandle::Task> create_workflow(Compression compression);

			std::string _name;
			std::string _author;
			std::string _description;
			size_t _version;
			std::vector<Dependency> _dependencies;

			std::unordered_map<int, std::pair<int, std::string>> _folders;
			std::vector<FileHandle> _file_handles;

			crypto::RSA _key;

			bool _signed = false;

			// Start of key/hash
			size_t _offset1;
			// Start of actual data
			size_t _offset2;

			Directory* _directory;
	};
};
