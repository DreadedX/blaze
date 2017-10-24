#pragma once

#include "archive.h"
#include "meta_asset.h"

namespace FLAME_NAMESPACE {
	class ArchiveWriter {
		public:
			ArchiveWriter(std::string name, std::string filename, std::string author, std::string description, uint16_t version, flame::Compression compression, std::vector<std::pair<std::string, uint16_t>> dependencies);

			void sign(std::array<uint8_t, 1217>& priv_key);

			void add(MetaAsset& meta_asset);
			void add_dependency(std::string name, uint16_t version);

		private:
			std::vector<MetaAsset::Task> create_workflow();

			std::shared_ptr<FileHandler> _fh;
			std::string _name;
			std::string _author;
			std::string _description;
			uint16_t _version;
			Compression _compression;
			std::vector<std::pair<std::string, uint16_t>> _dependencies;

			bool _signed = false;

	};
}
