#pragma once

#include <string>
#include <fstream>
#include <memory>

#define HASH_ALOGRITHM CryptoPP::SHA3_256
#define HASH_SIZE HASH_ALOGRITHM::DIGESTSIZE
#define PRIVATE_KEY_BIT_SIZE 2048
#define SIGNATURE_SIZE PRIVATE_KEY_BIT_SIZE/8
#define PUBLIC_KEY_SIZE PRIVATE_KEY_BIT_SIZE/8 + 36

namespace blaze::flame {
	class Asset;
	class ASyncFStream;

	// Archives only exist for writing files
	class Archive {
		public:
			Archive(std::shared_ptr<ASyncFStream> afs, std::string author, std::string description) : _afs(afs), _author(author), _description(description) {}

			void initialize();
			void finialize();

		private:
			std::string _author;
			std::string _description;
			std::shared_ptr<ASyncFStream> _afs;
			bool _signed = false;

		friend Archive& operator<<(Archive& archive, Asset& asset);
	};

	Archive& operator<<(Archive& archive, Asset& asset);
};
