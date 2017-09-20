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

			const std::string& get_name() const;
			uint16_t get_version() const;

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

			// @todo We need a better way of figuring out how to do workflows, they need to have some kind of base workflow, e.g. zlib decompress if coming from an archive and than the ability to add addition task on top of that
			// Pass Workflow in in constuctor, will be used as base and than workflow in get_data which append to the base workflow
			void set_workflow(Workflow _workflow);
			const Workflow& get_workflow() const;

		private:
			std::string _name;
			std::shared_ptr<ASyncFStream> _afs;
			uint16_t _version;
			uint32_t _offset;
			// The size of the data on disk, really only for internal use
			uint32_t _size;
			bool _chunk_markers;

			Workflow _workflow;
	};
};

