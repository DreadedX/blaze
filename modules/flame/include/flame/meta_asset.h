#pragma once

#include "flame.h"

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

			MetaAsset(std::string name, std::string filename, size_t version, size_t offset, size_t size, std::vector<Task> workflow = std::vector<Task>());
			MetaAsset(std::string name, std::string filename, size_t version, std::vector<Task> workflow) : MetaAsset(name, filename, version, 0, 0, workflow) {}

			const std::string& get_name() const;
			size_t get_version() const;

			AssetData get_data(bool async = true, std::vector<Task> workflow = std::vector<Task>(), std::function<void(std::vector<uint8_t>)> callback = nullptr);

		private:
			std::string _filename;
			std::string _name;
			size_t _version;
			size_t _offset;
			// The size of the data on disk, really only for internal use
			size_t _size;

			std::vector<Task> _base_workflow;
	};
};

