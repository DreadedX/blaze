#pragma once

#include "flame.h"
#include "flame/file_handler.h"
#include "flame/meta_asset.h"

#include "rsa.h"

namespace FLAME_NAMESPACE {

	enum class Compression : uint8_t {
		none,
		zlib
	};

	typedef std::tuple<std::string, size_t, size_t> Dependency;

	// FLMx is the new wip spec
	const std::vector<uint8_t> MAGIC = {'F','L','M','x'};
	// 1024 bit key
	const int KEY_SIZE = 1024/8;

	std::vector<uint8_t> calculate_hash(std::shared_ptr<FileHandler> fh, size_t size, size_t offset = 0);

	class MetaAsset;
	class FileHandler;
	class AssetList;

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
			std::vector<MetaAsset> get_meta_assets();

			// @note File stream automatically closes if the program ends, only if you explicitly need to close the archive
			void close() {
				_fh->close();
				_fh = nullptr;
			}

		protected:
			Archive(std::shared_ptr<FileHandler> fh, std::string name, std::string author, std::string description, size_t version, std::vector<Dependency> dependencies, crypto::RSA priv) : _fh(fh), _name(name), _author(author), _description(description), _version(version), _dependencies(dependencies), _priv(priv) {}

			std::vector<MetaAsset::Task> create_workflow(Compression compression);

			std::shared_ptr<FileHandler> _fh;
			std::string _name;
			std::string _author;
			std::string _description;
			size_t _version;
			std::vector<Dependency> _dependencies;

			std::vector<MetaAsset> _meta_assets;

			crypto::RSA _priv;

			bool _signed = false;

			// Start of key/hash
			size_t _offset1;
			// Start of actual data
			size_t _offset2;
	};
};
