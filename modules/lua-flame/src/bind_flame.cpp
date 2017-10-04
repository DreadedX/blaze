#include "bind_flame.h"

#include "archive.h"
#include "archive_writer.h"
#include "meta_asset.h"
#include "asset_data.h"

#include <string>
#include <memory>

namespace FLAME_NAMESPACE::lua {
	void bind(sol::state& lua) {
		lua.new_usertype<MetaAsset> ("MetaAsset",
			// @todo This should not be available in the game (constructors), MetaAssets in general are not really usefull in the game
			sol::constructors<
				MetaAsset(std::string, std::string, uint16_t, MetaAsset::Workflow),
				MetaAsset(std::string, std::shared_ptr<FileHandler>, uint16_t, MetaAsset::Workflow),
				MetaAsset(std::string, std::shared_ptr<FileHandler>, uint16_t, uint32_t, uint32_t, MetaAsset::Workflow)
			>(),
			"get_name", &MetaAsset::get_name,
			"get_version", &MetaAsset::get_version,
			"get_data", &MetaAsset::get_data
		);

		lua.new_usertype<MetaAsset::Workflow> ("Workflow");

		lua.new_usertype<Archive> ("Archive",
			sol::constructors<
				Archive(std::shared_ptr<FileHandler> fh)
			>(),
			"is_trusted", &Archive::is_trusted,
			"get_name", &Archive::get_name,
			"get_author", &Archive::get_author,
			"get_description", &Archive::get_description,
			"get_version", &Archive::get_version,
			"get_dependencies", &Archive::get_dependencies,
			"get_meta_assets", &Archive::get_meta_assets
		);

		// @todo This should not be available in the game
		lua.new_usertype<ArchiveWriter> ("ArchiveWriter",
			sol::constructors<
				ArchiveWriter(std::string, std::string, std::string, std::string, uint16_t),
				ArchiveWriter(std::string, std::shared_ptr<FileHandler> fh, std::string, std::string, uint16_t)
			>(),
			"add_dependency", &ArchiveWriter::add_dependency,
			"initialize", &ArchiveWriter::initialize,
			"finalize", &ArchiveWriter::finalize,
			"add", &ArchiveWriter::add
		);

		lua.new_usertype<FileHandler> ("FileHandler",
			"close", &FileHandler::close,
			"lock", &FileHandler::lock,
			"is_open", &FileHandler::is_open,
			"unlock", &FileHandler::unlock
		);
	}
}
