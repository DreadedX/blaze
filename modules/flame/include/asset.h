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
			Asset(std::string name, std::shared_ptr<ASyncFStream> afs, uint16_t version, uint32_t offset, uint32_t size, bool chunk_markers);

			const std::string& get_name() const { return _name; }
			uint16_t get_version() const { return _version; }

			ASyncData get_data();

			// @todo How do we pass additional data in and out of these functions
			struct Workflow {
				using Task = std::function<
					std::pair<
						std::unique_ptr<uint8_t[]>,
						uint32_t
					>(std::pair<
						std::unique_ptr<uint8_t[]>,
						uint32_t
							>)
					>;

				std::vector<Task> inner;
				std::vector<Task> outer;
			};

			void set_workflow(Workflow workflow);

		private:
			std::string _name;
			std::shared_ptr<ASyncFStream> _afs;
			uint16_t _version;
			uint32_t _offset;
			bool _chunk_markers;
			// The size of the data on disk, really only for internal use
			uint32_t _size;

			Workflow _workflow;
	};
};

