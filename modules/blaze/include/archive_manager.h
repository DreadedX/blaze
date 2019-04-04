#pragma once

#include "blaze.h"

#include "flame/file_handle.h"
#include "flame/archive.h"
#include "flame/data_handle.h"
#include "flame/vfs.h"

#include <unordered_map>

namespace BLAZE_NAMESPACE {

	class archive_manager {
		public:
			static flame::DataHandle load_data(std::string name, std::vector<flame::FileHandle::Task> tasks = std::vector<flame::FileHandle::Task>());
			static void add(flame::Archive& archive);
			static bool check_dependency(flame::Dependency dependency);
			static std::vector<flame::Dependency> missing_dependecies(flame::Archive& archive);
			static void debug_list_file_handles();

			static flame::Directory* _root;
			static flame::Directory* _resources;
		private:
			// @note Because we have a list of archives, we can actually completely reload all archives and assets
			// @note The GameAsset system in turn can than also reload all of its assets and now we can, on the fly reload all assets that we want
			// @note Debug interface should make it possible to selectively reload GameAssets, it will require a full Archive and Asset reload
			static std::vector<flame::Archive> _archives;
	};
}
