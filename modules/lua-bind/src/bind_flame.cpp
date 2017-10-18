#include "bind_flame.h"

#include "archive.h"
#include "archive_writer.h"
#include "meta_asset.h"
#include "asset_data.h"

#include <string>
#include <memory>

namespace FLAME_NAMESPACE::lua {
	void bind(sol::state& lua) {

		sol::table flame = lua.create_table("flame");

		flame.new_usertype<MetaAsset> ("MetaAsset",
			// @todo This should not be available in the game (constructors), MetaAssets in general are not really usefull in the game
			sol::constructors<
				MetaAsset(std::string, std::string, uint16_t, MetaAsset::Workflow),
				MetaAsset(std::string, std::shared_ptr<FileHandler>, uint16_t, MetaAsset::Workflow),
				MetaAsset(std::string, std::shared_ptr<FileHandler>, uint16_t, uint32_t, uint32_t, MetaAsset::Workflow)
			>(),
			"name", sol::property(&MetaAsset::get_name),
			"version", sol::property(&MetaAsset::get_version),
			"get_data", &MetaAsset::get_data
		);

		flame.new_usertype<MetaAsset::Workflow> ("Workflow",
			"tasks", &MetaAsset::Workflow::tasks
		);

		flame.new_usertype<Archive> ("Archive",
			sol::constructors<
				Archive(std::shared_ptr<FileHandler> fh)
			>(),
			"is_trusted", &Archive::is_trusted,
			"name", sol::property(&Archive::get_name),
			"author", sol::property(&Archive::get_author),
			"description", sol::property(&Archive::get_description),
			"version", sol::property(&Archive::get_version),
			"dependencies", sol::property(&Archive::get_dependencies),
			"meta_assets", sol::property(&Archive::get_meta_assets)
		);

		// @todo This should not be available in the game
		flame.new_usertype<ArchiveWriter> ("ArchiveWriter",
			sol::constructors<
				ArchiveWriter(std::string, std::shared_ptr<FileHandler> fh, std::string, std::string, uint16_t, Compression)
			>(),
			"add_dependency", &ArchiveWriter::add_dependency,
			"initialize", &ArchiveWriter::initialize,
			"finalize", &ArchiveWriter::finalize,
			"add", &ArchiveWriter::add
		);

		flame.new_enum("Compression",
			"none", Compression::none,
			"zlib", Compression::zlib
		);

		flame.new_usertype<FileHandler> ("FileHandler",
			"close", &FileHandler::close,
			"lock", &FileHandler::lock,
			"open", sol::property(&FileHandler::is_open),
			"unlock", &FileHandler::unlock
		);
	}
}
