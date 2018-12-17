#pragma once

#include "game_asset.h"

#if defined(__GNUC__) || defined (__clang__)
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#else
	#pragma warning(push, 0)
#endif
#include "sol.hpp"
#if defined(__GNUC__) || defined (__clang__)
	#pragma GCC diagnostic pop
#else
	#pragma warning(pop)
#endif

namespace BLAZE_NAMESPACE {

	class Script : public GameAsset {
		public:
			Script(std::string asset_name);
			~Script();

			bool is_loaded() override;

			void update();

		private:
			sol::environment environment;
			bool _loaded = false;
	};
}
