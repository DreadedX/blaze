#pragma once

#include "archive.h"
#include "meta_asset.h"

namespace FLAME_NAMESPACE {
	class ArchiveWriter {
		public:
			ArchiveWriter(std::string name, std::shared_ptr<FileHandler> fh, std::string author, std::string description, uint16_t version, flame::Compression compression);

			void initialize();
			void finalize(std::array<uint8_t, 1217>& priv_key);

			void add(MetaAsset& meta_asset);
			void add_dependency(std::string name, uint16_t version);

		private:
			MetaAsset::Workflow create_workflow();

			std::shared_ptr<FileHandler> _fh;
			std::string _name;
			std::string _author;
			std::string _description;
			uint16_t _version;
			std::vector<std::pair<std::string, uint16_t>> _dependencies;

			bool _initialized = false;
			bool _valid = false;

			Compression _compression;
	};
}
