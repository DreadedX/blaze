#pragma once

#include "flame/archive.h"
#include "flame/meta_asset.h"

namespace FLAME_NAMESPACE {
	class ArchiveWriter : public Archive {
		public:
			ArchiveWriter(std::string name, std::string filename, std::string author, std::string description, size_t version, std::vector<Dependency> dependencies, crypto::RSA priv = crypto::RSA(std::vector<uint8_t>(), std::vector<uint8_t>()));	

			void finalize();

			void add(MetaAsset& meta_asset, Compression compression);

		private:
			std::vector<MetaAsset::Task> create_workflow(Compression compression);

			bool _finalized = false;
	};
}
