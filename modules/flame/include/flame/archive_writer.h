#pragma once

#include "flame/archive.h"
#include "flame/meta_asset.h"

namespace FLAME_NAMESPACE {
	class ArchiveWriter {
		public:
			// @todo Make key optional
			ArchiveWriter(std::string name, std::string filename, std::string author, std::string description, uint16_t version, std::vector<Dependency> dependencies, crypto::RSA priv);	

			void sign();

			void add(MetaAsset& meta_asset, Compression compression);

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
			crypto::RSA _priv;

			bool _signed = false;

	};
}
