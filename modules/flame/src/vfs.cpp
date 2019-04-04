#include "flame/vfs.h"

#include <iostream>
#include <sstream>
#include <fstream>

namespace FLAME_NAMESPACE {
	Directory::Directory(std::string name, Directory* parent) : _name(name), _parent(parent) {}

	std::string Directory::get_name() const {
		return _name;
	}

	// This is more for testing
	std::string Directory::get_path() const {
		std::string path;

		if (_parent) {
			path += _parent->get_path() + '/';
		}
		path += _name;

		return path;
	}

	void Directory::add_directory(Directory* directory) {
		_directories.insert({directory->get_name(), directory});
	}

	// @todo This should really consume the directory
	void Directory::merge_directory(Directory* directory) {
		for (auto& [name, dir] : directory->_directories) {
			dir->_parent = this;
			add_directory(dir);
		}
		for (auto& [name, files] : directory->_files) {
			for (auto& [version, file] : files) {
				add_file(file);
			}
		}
	}

	void Directory::add_file(FileHandle file_handle) {
		std::string name = file_handle.get_name();
		auto iterator = _files.find(name);
		if (iterator == _files.end()) {
			_files.insert({name, std::map<size_t, FileHandle>()});
		}
		iterator = _files.find(name);
		// @todo If a file with the same version number already exist we just override it
		size_t version = file_handle.get_version();
		if (iterator->second.find(version) != iterator->second.end()) {
			std::cerr << "File with same version number is getting overriden\n";
		}
		iterator->second.insert({version, file_handle});
	}

	void Directory::test_tree() {
		for (auto& [name, directory] : _directories) {
			std::cout << "\e[34m" << directory->get_path() << "\e[0m\n";
			directory->test_tree();
		}
		for (auto& [name, file] : _files) {
			std::cout << get_path() << '/' << name << " (" << file.size() << ')' << '\n';
		}
	}

	FileHandle& Directory::get_file(std::string path, size_t version) {
		std::stringstream path_stream(path);
		std::string s;
		std::vector<std::string> parts;
		while (std::getline(path_stream, s, '/')) {
			parts.push_back(s);
		}

		return get_file(parts, 0, version);
	}

	FileHandle& Directory::get_file(std::vector<std::string> parts, size_t index, size_t version) {
		if (parts.size()-1 == index) {
			// This assumes it exists
			std::string name = parts[parts.size()-1];
			if (version == std::numeric_limits<size_t>::max()) {
				// @todo Beautiful looking code here
				return (--_files.find(name)->second.end())->second;
			} else {
				return _files.find(name)->second.at(version);
			}
		}

		std::string next_folder = parts[index];

		// Go back to root
		if (!next_folder.compare("") && index == 0) {
			if (_parent) {
				return _parent->get_file(parts, index, version);
			} else {
				return get_file(parts, index+1, version);
			}
		}

		// Go up one folder
		if (!next_folder.compare("..")) {
			if (_parent) {
				return _parent->get_file(parts, index+1, version);
			} else {
				throw std::runtime_error("No parent folder");
			}
		}

		// Stay in the current folder
		if (!next_folder.compare(".")) {
			return get_file(parts, index+1, version);
		}

		return _directories[next_folder]->get_file(parts, index+1, version);
	}

	Directory* Directory::get_directory(std::string name) {
		auto iterator = _directories.find(name);
		if (iterator == _directories.end()) {
			return nullptr;
		}
		return iterator->second;
	}

	// This is just a prove of concept version
	// @todo We should probably have a better name here
	class Provider {
		public:
			virtual std::unique_ptr<std::istream> get_stream() = 0;
	};

	class DiskProvider : public Provider {
		public:
			DiskProvider(std::string path) : _path(path) {}

			std::unique_ptr<std::istream> get_stream() override {
				return std::make_unique<std::ifstream>(_path);
			}

		private:
			std::string _path;
	};

	class VfsProvider : public Provider {
		public:
			VfsProvider(std::string path, Directory* root) : _path(path), _root(root) {}

			std::unique_ptr<std::istream> get_stream() override {
				return _root->get_file(_path).load_data(false).get_as<std::unique_ptr<std::istream>>();
			}

		private:
			std::string _path;
			Directory* _root;
	};

	// @todo This is another example of something we could do
	// class WebProvider : public Provider {
	// 	public:
	// 		WebProvider(std::string url) : _url(url) {}
    //
	// 		std::unique_ptr<std::istream> get_stream() override {
	// 			// @todo Fetch the file from a online resource
	// 		};
    //
	// 	private:
	// 		std::string _url;
	// };

	void vfs_test() {

		auto root = new Directory();

		auto res = new Directory("resources", root);
		root->add_directory(res);
		auto base = new Directory("base", res);
		res->add_directory(base);

		auto saves = new Directory("saves", root);
		root->add_directory(saves);

		// @todo We want to replace FileInfo with a backend system
		// Instead of a path that will be opened using fstream we want a provider that returns a stream
		// Examples for backends:
		//		Disk
		//		Other vfs file
		//		http(s)
		base->add_file(FileHandle("Script", 1, internal::FileInfo{"/home/tim/Projects/cpp/blaze/assets/base/script/Script.lua", 0, 524}));
		saves->add_file(FileHandle("CVars", 1, internal::FileInfo{"/home/tim/Projects/cpp/blaze/assets/base/script/Script.lua", 0, 524}));
		saves->add_file(FileHandle("CVars", 1, internal::FileInfo{"/home/tim/Projects/cpp/blaze/.flint/build/linux/debug/archives/cvars.txt", 0, 10}));
		saves->add_file(FileHandle("log", 1, internal::FileInfo{"/home/tim/Projects/cpp/blaze/blaze.verbose.log", 0, 132}));
		saves->add_file(FileHandle("log", 2, internal::FileInfo{"/home/tim/Projects/cpp/blaze/blaze.log", 0, 40}));

		root->test_tree();

		auto script = root->get_file("/saves/../resources/base/Script").load_data(false).get_as<std::string>();
		std::cout << script << '\n';

		auto cvars = saves->get_file("./CVars").load_data(false).get_as<std::string>();
		std::cout << cvars << '\n';

		auto log = root->get_file("/saves/log").load_data(false).get_as<std::string>();
		std::cout << log << '\n';

		auto log1 = root->get_file("/saves/log", 1).load_data(false).get_as<std::string>();
		std::cout << log1 << '\n';

		auto log2 = root->get_file("/saves/log", 2).load_data(false).get_as<std::string>();
		std::cout << log2 << '\n';

		{
			Provider *disk_provider = new DiskProvider("/home/tim/Projects/cpp/blaze/webserver/package.json");
			auto test = disk_provider->get_stream();
			std::cout << test->rdbuf() << '\n';
		}

		{
			Provider *vfs_provider = new VfsProvider("/saves/CVars", root);
			auto test = vfs_provider->get_stream();
			std::cout << test->rdbuf() << '\n';
		}


		// Example game engine usage
		// First we use disk_provider (or https_provider) to load all archives in /archives
		// Then we can call a command that takes an vfs path and target directory (e.g. /resources)
		// It will then create the directory structure of the archive file within that directory
		// Each file here will use the vfs_provider pointing to the archive in /archives
		// This requires that we store the offset somewhere...
		// Or we implement a special provider that is vfs_provider but with offset and size specified
	}
}
