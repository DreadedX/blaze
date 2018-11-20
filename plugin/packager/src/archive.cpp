#include "packager/archive.h"
#include "flame/archive_writer.h"
#include "flint.h"
#include "logger.h"

#include "flint/helper.h"

#include "rsa.h"

// @todo We need to make sure that the 'archives' folder gets created

Archive::Archive(Flint& flint, Config config, std::string name) : Data(flint, config, name), _flint(flint) {
	_flint.register_global("author");
	_functions["author"] = std::bind(&Archive::author, this, std::placeholders::_1);
	_flint.register_global("description");
	_functions["description"] = std::bind(&Archive::description, this, std::placeholders::_1);
	_flint.register_global("version");
	_functions["version"] = std::bind(&Archive::version, this, std::placeholders::_1);
	_flint.register_global("compression");
	_functions["compression"] = std::bind(&Archive::compression, this, std::placeholders::_1);
	_flint.register_global("key");
	_functions["key"] = std::bind(&Archive::key, this, std::placeholders::_1);
	_flint.register_global("path");
	_functions["path"] = std::bind(&Archive::path, this, std::placeholders::_1);
	_flint.register_global("requires");
	_functions["task"] = std::bind(&Archive::task, this, std::placeholders::_1);
	_flint.register_global("task");
	_functions["requires"] = std::bind(&Archive::requires, this, std::placeholders::_1);
	_flint.register_global("version_min");
	_functions["version_min"] = std::bind(&Archive::version_min, this, std::placeholders::_1);
	_flint.register_global("version_max");
	_functions["version_max"] = std::bind(&Archive::version_max, this, std::placeholders::_1);
	_flint.register_global("script");
	_functions["script"] = std::bind(&Archive::script, this, std::placeholders::_1);

	_flint.register_global("asset");
	_functions["asset"] = std::bind(&Archive::asset, this, std::placeholders::_1);

	_path = "/archives/" + name + ".flm";
}

void Archive::build() {
	
	helper::create_directory(_config.build_path + '/' + "archives");

	// Convert the dependecies to actual dependencies
	std::vector<flame::Dependency> dependencies;
	for (auto& dependency : _dependencies) {
		dependencies.push_back(flame::Dependency({dependency.name, dependency.min_version, dependency.max_version}));
	}

	if (_author.empty()) {
		_author = "(unknown)";
	}
	if (_description.empty()) {
		_description = "(none)";
	}

	auto archive_writer = [=] {
		if (!_key.empty()) {
			auto [pub, priv] = crypto::load(_key);
			return flame::ArchiveWriter(get_name(), _config.build_path + _path, _author, _description, _version, dependencies, priv);
		} else {
			return flame::ArchiveWriter(get_name(), _config.build_path + _path, _author, _description, _version, dependencies);
		}
	}();

	for (auto& asset : _assets) {
		// for (flame::MetaAsset::Task task : asset.tasks) {
		// 	LOG_D("FUCK\n");
		// 	std::vector<uint8_t> data = { 0x00, 0x01 };
		// 	data = task(data);
		// 	LOG_D("FUCKER\n");
		// }

		flame::MetaAsset meta_asset(asset.name, asset.path, asset.version, asset.tasks);
		// flame::MetaAsset meta_asset(asset.name, asset.path, asset.version);
		archive_writer.add(meta_asset, asset.compression);
	}

	archive_writer.finalize();
}

void Archive::author(sol::variadic_args args) {
	if (args.size() < 1) {
		LOG_E("A least one author needs to be given.\n");
		return;
	}
	if (_context_type != ContextType::none) {
		LOG_E("Author is only available in the context of an archive\n");
		return;
	}

	for (std::string author : args) {
		if (!_author.empty()) {
			_author += " & ";
		}
		_author += author;
	}

	// LOG_D("Author: {}\n", _author);
}

void Archive::description(sol::variadic_args args) {
	if (args.size() != 1) {
		LOG_E("Description takes one argument.\n");
		return;
	}
	if (_context_type != ContextType::none) {
		LOG_E("Description is only available in the context of an archive\n");
		return;
	}

	_description = args.get<std::string>(0);

	// LOG_D("Description: {}\n", _description);
}

// @todo I notice the same pattern over and over again, would be nice to put this a template of sorts
void Archive::version(sol::variadic_args args) {
	if (args.size() != 1) {
		LOG_E("Version takes one argument.\n");
		return;
	}

	auto version = args.get<size_t>(0);

	switch (_context_type) {
		case ContextType::none: {
			_version = version;
			// LOG_D("Version: {}\n", _version);
			break;
		}
		case ContextType::asset: {
			_assets[_context].version = version;
			// LOG_D("Version: {} (Asset: {})\n", _assets[_context].version, _assets[_context].name);
			break;
		}
		default: {
			LOG_E("Version is only available in the context of an archive or an asset\n");
			break;
		}
	}
}

void Archive::compression(sol::variadic_args args) {
	if (args.size() != 1) {
		LOG_E("Compression takes one argument.\n");
		return;
	}

	flame::Compression compression = args.get<flame::Compression>(0);

	switch (_context_type) {
		case ContextType::none: {
			_compression = compression;
			// LOG_D("Compression: {}\n", (uint8_t)_compression);
			break;
		}
		case ContextType::asset: {
			_assets[_context].compression = compression;
			// LOG_D("Compression: {} (Asset: {})\n", (uint8_t)_assets[_context].compression, _assets[_context].name);
			break;
		}
		default: {
			LOG_E("Description is only available in the context of an archive or an asset\n");
			break;
		}
	}
}

void Archive::key(sol::variadic_args args) {
	if (args.size() != 1) {
		LOG_E("Key takes one argument.\n");
		return;
	}
	if (_context_type != ContextType::none) {
		LOG_E("Key is only available in the context of an archive\n");
		return;
	}

	_key = args.get<std::string>(0);
	// LOG_D("Key: {}\n", _key);
}

// @todo Maybe provide a nice default for the path, baecause right now we can make an asset that has now path which is weird
// Maybe set a base folder in the archive and append the name to the base path?
// Or if an asset has no path we assume that one of the tasks generates something
void Archive::asset(sol::variadic_args args) {
	if (args.size() != 1) {
		LOG_E("Asset takes one argument.\n");
		return;
	}

	_assets.push_back(Asset());
	_context = _assets.size()-1;
	_context_type = ContextType::asset;

	_assets[_context].name = args.get<std::string>(0);
	_assets[_context].version = _version;
	_assets[_context].compression = _compression;

	// LOG_D("Asset: {}\n", _assets[_context].name);
}

void Archive::path(sol::variadic_args args) {
	if (args.size() != 1) {
		LOG_E("Asset takes one argument.\n");
		return;
	}
	if (_context_type != ContextType::asset) {
		LOG_E("Path is only available in the context of an asset\n");
		return;
	}

	_assets[_context].path = args.get<std::string>(0);
	// LOG_D("Path: {} (Asset: {})\n", _assets[_context].path, _assets[_context].name);
}

void Archive::task(sol::variadic_args args) {
	if (args.size() == 0) {
		LOG_E("Task takes at least one argument.\n");
		return;
	}
	if (_context_type != ContextType::asset) {
		LOG_E("Task is only available in the context of an asset\n");
		return;
	}

	for (sol::function func : args) {
		_assets[_context].tasks.push_back(flame::MetaAsset::Task(func));
	}
}

void Archive::requires(sol::variadic_args args) {
	if (args.size() != 1) {
		LOG_E("Dependency takes one argument.\n");
		return;
	}

	_dependencies.push_back(Dependency());
	_context = _dependencies.size()-1;
	_context_type = ContextType::dependency;

	_dependencies[_context].name = args.get<std::string>(0);
	_dependencies[_context].min_version = 0;
	_dependencies[_context].max_version = 0;

	// LOG_D("Requires: {}\n", _dependencies[_context].name);
}

void Archive::version_max(sol::variadic_args args) {
	if (args.size() != 1) {
		LOG_E("Asset takes one argument.\n");
		return;
	}
	if (_context_type != ContextType::dependency) {
		LOG_E("Min Version is only available in the context of a dependency\n");
		return;
	}

	_dependencies[_context].min_version = args.get<size_t>(0);
	// LOG_D("Min Version: {} (Dependency: {})\n", _dependencies[_context].min_version, _dependencies[_context].name);
}

void Archive::version_min(sol::variadic_args args) {
	if (args.size() != 1) {
		LOG_E("Asset takes one argument.\n");
		return;
	}
	if (_context_type != ContextType::dependency) {
		LOG_E("Max Version is only available in the context of a dependency\n");
		return;
	}

	_dependencies[_context].max_version = args.get<size_t>(0);
	// LOG_D("Max Version: {} (Dependency: {})\n", _dependencies[_context].max_version, _dependencies[_context].name);
}

void Archive::script(sol::variadic_args args) {
	if (args.size() != 1) {
		LOG_E("Description takes one argument.\n");
		return;
	}
	if (_context_type != ContextType::none) {
		LOG_E("Description is only available in the context of an archive\n");
		return;
	}

	_flint.call("asset", get_name() + "/Script");
	_flint.call("path", args.get<std::string>(0));
	_context_type = ContextType::none;
}
