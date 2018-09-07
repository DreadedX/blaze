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

	typedef std::tuple<std::string, uint16_t, uint16_t> Dependency;

	constexpr uint8_t MAGIC[] = {'F','L','M','b'};
	// 1024 bit key
	const int KEY_SIZE = 1024/8;

	std::vector<uint8_t> calculate_hash(std::shared_ptr<FileHandler> fh, uint32_t size);

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
			const uint16_t& get_version() const;
			bool is_trusted(crypto::RSA& trusted_key);
			const std::vector<Dependency>& get_dependencies() const;
			std::vector<MetaAsset> get_meta_assets();

			// @note File stream automatically closes if the program ends, only if you explicitly need to close the archive
			void close() {
				_fh->close();
				_fh = nullptr;
			}

		private:
			std::vector<MetaAsset::Task> create_workflow(Compression compression);

			std::shared_ptr<FileHandler> _fh;
			std::string _name;
			std::string _author;
			std::string _description;
			uint16_t _version;
			std::vector<Dependency> _dependencies;

			std::vector<MetaAsset> _meta_assets;

			crypto::RSA _key;
	};
};
