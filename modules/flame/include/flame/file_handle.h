#pragma once

#include "flame.h"

#include "flame/data_loader.h"

#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace FLAME_NAMESPACE {
	namespace internal {
		struct FileInfo {
			std::string filename;
			size_t offset = 0;
			size_t size = 0;
		};
	}

	class FileHandle {
		public:
			typedef std::function<std::vector<uint8_t>(std::vector<uint8_t>)> Task;

			FileHandle(std::string name, size_t version, internal::FileInfo file_info, std::vector<Task> workflow = std::vector<Task>());

			const std::string& get_name() const;
			size_t get_version() const;

			DataLoader get_data(bool async = true, std::vector<Task> workflow = std::vector<Task>());

		private:
			std::vector<uint8_t> async_load(std::vector<Task> workflow);

			internal::FileInfo _file_info;

			std::string _name;
			size_t _version;

			std::vector<Task> _base_workflow;
	};
};

