#pragma once

#include "logger.h"

#include "blaze.h"

#include "flame/asset_data.h"
#include "asset_list.h"
#include "engine.h"

#include <string>
#include <string_view>
#include <functional>

namespace BLAZE_NAMESPACE {

	class GameAssetBase {
		public:
			GameAssetBase(std::string asset_name);
			virtual ~GameAssetBase() {}

			virtual bool is_loaded() = 0;

			std::string_view get_name() const;

		private:
			std::string _name;
	};

	class GameAssetLoaded : public GameAssetBase {
		public:
			GameAssetLoaded(std::string asset_name, std::vector<flame::MetaAsset::Task> tasks = std::vector<flame::MetaAsset::Task>());
			virtual ~GameAssetLoaded() {}

			bool is_loaded() override;

		protected:
			flame::AssetData _data;
	};
}
