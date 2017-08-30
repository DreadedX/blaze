#pragma once

#include <memory>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <functional>

#include "async_data.h"
#include "async_fstream.h"

namespace blaze::flame {
	class Archive;
	class Asset {
		public:
			// @todo This constructor is only for testing
			Asset(std::string name, std::shared_ptr<ASyncFStream> afs, uint8_t version, uint32_t offset = 0) : _name(name), _afs(afs), _version(version), _offset(offset) {
				if (_afs && _afs->is_open()) {
					auto& fs = _afs->lock();
					auto pos = fs.tellg();
					fs.seekg(0, std::ios::end);
					_size = fs.tellg();
					fs.seekg(pos);
					_afs->unlock();
				} else {
					std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "File stream closed\n";
					_size = 0;
				}
			}

			~Asset() {
				// @note Make sure all sub threads are finished before continuing
				_mutex.lock();
			}

			std::shared_ptr<ASyncData> get_data();

			// @todo Clear out finished threads every once in a while
			static void wait_for_workers() {
				for(auto& t : _workers) {
					t.join();
				};
			}

			using TaskData = std::pair<std::shared_ptr<uint8_t[]>, uint32_t>;
			// @todo How do we pass additional data in and out of these functions
			void add_load_task(std::function< TaskData(TaskData)> task) {
				_tasks.push_back(task);
			}

		private:
			std::string _name;
			std::shared_ptr<ASyncFStream> _afs;
			uint8_t _version;
			uint32_t _offset;
			// The size of the data on disk, really only for internal use
			uint32_t _size;

			std::weak_ptr<ASyncData> _async_data;

			std::vector<std::function<TaskData(TaskData)>> _tasks;

			std::mutex _mutex;

			static std::vector<std::thread> _workers;

			friend Archive& operator<<(Archive& archive, Asset& asset);
	};
};

