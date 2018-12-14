#pragma once

#include "flame/archive.h"
#include "flame/file_handle.h"

#include <fstream>

namespace FLAME_NAMESPACE {
	class ArchiveWriter : public Archive {
		public:
			// @todo Make sure we add all assets to the _file_handles list
			ArchiveWriter(std::string name, std::string filename, std::string author, std::string description, size_t version, std::vector<Dependency> dependencies, crypto::RSA priv = crypto::RSA(std::vector<uint8_t>(), std::vector<uint8_t>()));	

			void finalize();

			void add(FileHandle& file_handle, Compression compression);

		private:
			std::vector<FileHandle::Task> create_workflow(Compression compression);

			std::fstream _fs;

			bool _finalized = false;
	};
}
