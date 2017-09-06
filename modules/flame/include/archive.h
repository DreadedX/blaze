#pragma once

#include "binary_helper.h"

#include <string>
#include <fstream>
#include <memory>

#define HASH_ALOGRITHM CryptoPP::SHA3_256
#define HASH_SIZE HASH_ALOGRITHM::DIGESTSIZE
#define PRIVATE_KEY_BIT_SIZE 2048
#define SIGNATURE_SIZE PRIVATE_KEY_BIT_SIZE/8
#define PUBLIC_KEY_SIZE PRIVATE_KEY_BIT_SIZE/8 + 36

namespace blaze::flame {

	constexpr uint8_t MAGIC[] = {'F','L','M','b'};

	class Asset;
	class ASyncFStream;
	class AssetList;

	// Archives only exist for writing files
	class Archive {
		public:
			Archive(std::shared_ptr<ASyncFStream> afs, std::string author, std::string description);
			Archive(std::shared_ptr<ASyncFStream> afs);
			void initialize();
			void finialize(std::shared_ptr<uint8_t[]> priv_key, uint32_t key_size);

			// Check if the archive is a trusted archive by comparing the key with a trusted key
			bool is_trusted(uint8_t trusted_key[]) { return binary::compare(_key, trusted_key, PUBLIC_KEY_SIZE); };
			const std::string& get_author() const { return _author; }
			const std::string& get_description() const { return _description; }

			const std::shared_ptr<ASyncFStream> get_async_fstream() const { return _afs; }

		private:
			std::shared_ptr<ASyncFStream> _afs;
			std::string _author;
			std::string _description;

			bool _initialized = false;
			bool _valid = false;
			uint8_t _key[PUBLIC_KEY_SIZE];

		friend Archive& operator<<(Archive& archive, Asset& asset);
		friend AssetList& operator<<(AssetList& asset_list, Archive& archive);
	};

	Archive& operator<<(Archive& archive, Asset& asset);
	// @todo This should propably be in asset_list
	AssetList& operator<<(AssetList& asset_list, Archive& archive);
};
