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
			typedef std::function<std::vector<uint8_t>(std::vector<uint8_t>)> Task;

			// @todo Ehm... We somehow need this to make the asset list work
			MetaAsset() {}

			MetaAsset(std::string name, std::string filename, uint16_t version, std::vector<Task> workflow = std::vector<Task>());
			MetaAsset(std::string name, std::shared_ptr<FileHandler> fh, uint16_t version, uint32_t offset, uint32_t size, std::vector<Task> workflow = std::vector<Task>());

			const std::string& get_name() const;
			uint16_t get_version() const;

			AssetData get_data(std::vector<Task> workflow = std::vector<Task>());

		private:
			std::string _name;
			std::shared_ptr<FileHandler> _fh;
			uint16_t _version;
			uint32_t _offset;
			// The size of the data on disk, really only for internal use
			uint32_t _size;

			std::vector<Task> _base_workflow;
	};
};

