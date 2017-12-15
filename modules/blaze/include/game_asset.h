#pragma once

#include "logger.h"

#include "blaze.h"

#include "asset_data.h"
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
				auto it = _strings.find(name);
				if (it == _strings.end()) {
					return "(undefined)";
				}

				return fmt::format(it->second, args...);
			}
		private:
			void post_load();

			std::unordered_map<std::string, std::string> _strings;
	};
}
