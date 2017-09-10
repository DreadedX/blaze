#pragma once

#include "async_fstream.h"

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <future>

namespace blaze::flame {
	class Archive;
	class ASyncData;
	// @todo This is not really and asset anymore, it is just the MetaData, ASync data is the actual data. Maybe AssetMeta and AssetData
	class Asset {
		public:
			// @todo Ehm... We somehow need this to make the asset list work
			Asset() {}

			Asset(std::string name, std::shared_ptr<ASyncFStream> afs, uint16_t version);
			Asset(std::string name, std::shared_ptr<ASyncFStream> afs, uint16_t version, uint32_t offset, uint32_t size);

			const std::string& get_name() const { return _name; }
			uint16_t get_version() const { return _version; }

			ASyncData get_data();
			// @todo Maybe a better name than TaskData
			using TaskData = std::pair<std::unique_ptr<uint8_t[]>, uint32_t>;
			// @todo How do we pass additional data in and out of these functions
			void add_load_task(std::function< TaskData(TaskData)> task);

		private:
			std::string _name;
			std::shared_ptr<ASyncFStream> _afs;
			uint16_t _version;
			uint32_t _offset;
			// The size of the data on disk, really only for internal use
			uint32_t _size;

			std::vector<std::function<TaskData(TaskData)>> _tasks;
	};
};

