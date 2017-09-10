#pragma once

#include "asset.h"
#include "archive.h"
#include "async_data.h"

#include <unordered_map>

namespace blaze::flame {
	class AssetList {
		public:
			ASyncData find_asset(std::string name);
			void add(Archive& archive);
			bool check_dependency(std::pair<std::string, uint16_t> dependency);
			void load();
			void debug_list_assets();

		private:
			std::vector<Archive> _archives;
			std::unordered_map<std::string, Asset> _assets;
	};
}
