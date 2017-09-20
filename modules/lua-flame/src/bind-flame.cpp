#include "bind-flame.h"

#include "archive.h"
#include "asset.h"
#include "async_data.h"

#include <string>
#include <memory>

namespace blaze::flame::lua {
	void bind(sol::state& lua) {
		lua.new_usertype<Asset> ("Asset",
			sol::constructors<
				Asset(std::string, std::shared_ptr<ASyncFStream>, uint16_t),
				Asset(std::string, std::shared_ptr<ASyncFStream>, uint16_t, uint32_t, uint32_t, bool)
			>(),
			"get_name", &Asset::get_name,
			"get_version", &Asset::get_version,
			// @todo Test if this works
			"get_data", &Asset::get_data
			// add_load_task
		);

		lua.new_usertype<Archive> ("Archive",
			sol::constructors<
				Archive(std::shared_ptr<ASyncFStream> afs, std::string, std::string, std::string, uint16_t),
				Archive(std::shared_ptr<ASyncFStream> afs)
			>(),
			"add_dependency", &Archive::add_dependency,
			"initialize", &Archive::initialize,
			"finalize", &Archive::finialize,
			"add", &Archive::add,
			"is_trusted", &Archive::is_trusted,
			"is_valid", &Archive::is_valid,
			"get_name", &Archive::get_name,
			"get_author", &Archive::get_author,
			"get_description", &Archive::get_description,
			"get_version", &Archive::get_version,
			"get_dependencies", &Archive::get_dependencies,
			"get_assets", &Archive::get_assets
		);

		// @todo Test the functionality
		lua.new_usertype<ASyncFStream> ("ASyncFStream",
			// This needs to be called if done, because garbage collector
			"close", &ASyncFStream::close,
			"lock", &ASyncFStream::lock,
			"is_open", &ASyncFStream::is_open,
			"unlock", &ASyncFStream::unlock
		);
	}
}
