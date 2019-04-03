#pragma once

#include "flame.h"

#include "flame/file_handle.h"

#include <map>

namespace FLAME_NAMESPACE {

	void vfs_test();

	class Directory {
		public:
			Directory(std::string name = "", Directory* parent = nullptr);

			std::string get_name() const;

			// This is more for testing
			std::string get_path() const;

			void add_directory(Directory* directory);

			void add_file(FileHandle file_handle);

			void test_tree();

			FileHandle get_file(std::string path, size_t version = std::numeric_limits<size_t>::max());

			FileHandle get_file(std::vector<std::string> parts, size_t index, size_t version);

		private:
			std::string _name;
			Directory* _parent = nullptr;

			std::unordered_map<std::string, Directory*> _directories;
			std::unordered_map<std::string, std::map<size_t, FileHandle>> _files;
	};
}
