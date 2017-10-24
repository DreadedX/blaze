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
				MetaAsset(std::string, std::string, uint16_t, std::vector<MetaAsset::Task>),
				MetaAsset(std::string, std::shared_ptr<FileHandler>, uint16_t, uint32_t, uint32_t, std::vector<MetaAsset::Task>)
			>(),
			"name", sol::property(&MetaAsset::get_name),
			"version", sol::property(&MetaAsset::get_version),
			"get_data", &MetaAsset::get_data
		);

		flame.set_function(
			"new_workflow", []{
				return std::vector<MetaAsset::Task>();
			}
		);

		flame.new_usertype<Archive> ("Archive",
			sol::constructors<
				Archive(std::string)
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
				ArchiveWriter(std::string, std::string, std::string, std::string, uint16_t, Compression, std::vector<std::pair<std::string, uint16_t>>)
			>(),
			"sign", &ArchiveWriter::sign,
			"add", &ArchiveWriter::add
		);

		flame.set_function(
			"new_dependency_list", []{
				return std::vector<std::pair<std::string, uint16_t>>();
			}
		);

		flame.new_enum("Compression",
			"none", Compression::none,
			"zlib", Compression::zlib
		);
	}
}
