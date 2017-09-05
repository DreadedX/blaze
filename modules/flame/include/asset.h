#pragma once

#include "async_data.h"
#include "async_fstream.h"

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <future>

namespace blaze::flame {
	class Archive;
	class Asset {
		public:
			// @todo This constructor is only for testing
			Asset(std::string name, std::shared_ptr<ASyncFStream> afs, uint8_t version);
			Asset(std::string name, std::shared_ptr<ASyncFStream> afs, uint8_t version, uint32_t offset, uint32_t size);

			const std::string get_name() const { return _name; }
			uint8_t get_version() const { return _version; }

			std::shared_ptr<ASyncData> get_data();
			// @todo Maybe a better name than TaskData
			using TaskData = std::pair<std::shared_ptr<uint8_t[]>, uint32_t>;
			// @todo How do we pass additional data in and out of these functions
			void add_load_task(std::function< TaskData(TaskData)> task);

		private:
			std::string _name;
			std::shared_ptr<ASyncFStream> _afs;
			uint8_t _version;
			uint32_t _offset;
			// The size of the data on disk, really only for internal use
			uint32_t _size;
			std::weak_ptr<ASyncData> _async_data;

			std::vector<std::function<TaskData(TaskData)>> _tasks;
			std::future<void> _future;

			friend Archive& operator<<(Archive& archive, Asset& asset);
	};
};

