#pragma once

#include "logger.h"

#include "blaze.h"

#include "flame/asset_data.h"
#include "asset_list.h"
#include "engine.h"

namespace BLAZE_NAMESPACE {

	class GameAsset {
		public:
			GameAsset(std::string asset_name);
			virtual ~GameAsset() {}

			bool is_loaded();
			static bool finish_if_loaded(std::shared_ptr<GameAsset> asset);

			const std::string& get_name() const;

		protected:
			flame::AssetData _data;
			std::string _name;
			
		private:
			virtual void post_load() = 0;
	};
}
