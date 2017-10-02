#pragma once

#include "flame.h"

#include "file_handler.h"

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <future>

namespace FLAME_NAMESPACE {
	class Archive;
	class AssetData;
	class MetaAsset {
		public:
			// @todo Ehm... We somehow need this to make the asset list work
			MetaAsset() {}

			struct Workflow {
				std::vector<
					std::function<
						std::vector<uint8_t>
						(
						std::vector<uint8_t>
						)
					>
				> tasks;
			};

			MetaAsset(std::string name, std::shared_ptr<FileHandler> fh, uint16_t version, Workflow workflow = Workflow());
			MetaAsset(std::string name, std::shared_ptr<FileHandler> fh, uint16_t version, uint32_t offset, uint32_t size, Workflow workflow = Workflow());

			const std::string& get_name() const;
			uint16_t get_version() const;

			AssetData get_data(Workflow workflow = Workflow());

		private:
			std::string _name;
			std::shared_ptr<FileHandler> _fh;
			uint16_t _version;
			uint32_t _offset;
			// The size of the data on disk, really only for internal use
			uint32_t _size;

			Workflow _base_workflow;
	};
};

