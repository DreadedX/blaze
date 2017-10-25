#pragma once

#include "flame.h"
#include "file_handler.h"
#include "meta_asset.h"

#include "sha3.h"

namespace FLAME_NAMESPACE {

	enum class Compression: uint8_t {
		none,
		zlib
	};

	constexpr uint8_t MAGIC[] = {'F','L','M','b'};
	typedef CryptoPP::SHA3_256 HASH_ALOGRITHM;
	const int HASH_SIZE = HASH_ALOGRITHM::DIGESTSIZE;
	const int PRIVATE_KEY_BIT_SIZE = 2048;
	const int SIGNATURE_SIZE = PRIVATE_KEY_BIT_SIZE/8;
	const int PUBLIC_KEY_SIZE = PRIVATE_KEY_BIT_SIZE/8 + 36;

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
			bool is_trusted(uint8_t trusted_key[]);
			const std::vector<std::pair<std::string, uint16_t>>& get_dependencies() const;
			std::vector<MetaAsset> get_meta_assets();

		private:
			std::vector<MetaAsset::Task> create_workflow();

			std::shared_ptr<FileHandler> _fh;
			std::string _name;
			std::string _author;
			std::string _description;
			uint16_t _version;
			Compression _compression;
			std::vector<std::pair<std::string, uint16_t>> _dependencies;

			std::vector<MetaAsset> _meta_assets;

			uint8_t _key[PUBLIC_KEY_SIZE];

	};
};
