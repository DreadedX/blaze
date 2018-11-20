#pragma once

#include "logger.h"

#include "blaze.h"

#include "flame/asset_data.h"
#include "asset_list.h"
#include "engine.h"

#include "lang.h"

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


	class Script : public GameAsset {
		public:
			Script(std::string asset_name);
			~Script();

			void update();

		private:
			void post_load() override;

			sol::environment environment;
			bool _loaded = false;
	};

	class Language : public GameAsset {
		public:
			Language(std::string asset_name);

			template <typename... Args>
			std::string get(std::string name, Args... args) {
				std::string value = _root.get_value(name);

				return fmt::format(value, args...);
			}
		private:
			void post_load();

			lang::Node _root;
	};
}
