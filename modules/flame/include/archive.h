#pragma once

#include "binary_helper.h"

#include "sha3.h"

#include <string>
#include <fstream>
#include <memory>

namespace blaze::flame {

	constexpr uint8_t MAGIC[] = {'F','L','M','b'};
	typedef CryptoPP::SHA3_256 HASH_ALOGRITHM;
	const int HASH_SIZE = HASH_ALOGRITHM::DIGESTSIZE;
	const int PRIVATE_KEY_BIT_SIZE = 2048;
	const int SIGNATURE_SIZE = PRIVATE_KEY_BIT_SIZE/8;
	const int PUBLIC_KEY_SIZE = PRIVATE_KEY_BIT_SIZE/8 + 36;

	class Asset;
	class ASyncFStream;
	class AssetList;

	// Archives only exist for writing files
	class Archive {
		public:
			Archive(std::shared_ptr<ASyncFStream> afs);
			Archive(std::shared_ptr<ASyncFStream> afs, std::string name, std::string author, std::string description, uint16_t version);

			void initialize();
			void finialize(std::array<uint8_t, 1217>& priv_key);

			void add(Asset& asset);
			void add_dependency(std::string name, uint16_t version);

			const bool& is_valid() const;
			bool is_trusted(uint8_t trusted_key[]);

			const std::string& get_name() const;
			const std::string& get_author() const;
			const std::string& get_description() const;
			const uint16_t& get_version() const;
			const std::vector<std::pair<std::string, uint16_t>>& get_dependencies() const;

			std::vector<Asset> get_assets();

		private:
			std::shared_ptr<ASyncFStream> _afs;
			std::string _name;
			std::string _author;
			std::string _description;
			uint16_t _version;
			std::vector<std::pair<std::string, uint16_t>> _dependencies;

			bool _initialized = false;
			bool _valid = false;
			uint8_t _key[PUBLIC_KEY_SIZE];
	};
};
