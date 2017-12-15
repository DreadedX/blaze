#pragma once

#include "archive.h"
#include "meta_asset.h"

namespace FLAME_NAMESPACE {
	class ArchiveWriter {
		public:
			ArchiveWriter(std::string name, std::string filename, std::string author, std::string description, uint16_t version, std::vector<std::tuple<std::string, uint16_t, uint16_t>> dependencies);

			void sign(crypto::RSA& priv_key);

			void add(MetaAsset& meta_asset, Compression compression);
			void add_dependency(std::string name, uint16_t version);

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
			std::vector<std::tuple<std::string, uint16_t, uint16_t>> _dependencies;

			bool _signed = false;

	};
}
