#pragma once
#include "flint/data.h"
#include "flame/archive.h"

// @todo Add tasks
struct Asset {
	std::string name;
	std::string path;
	size_t version;

	flame::Compression compression;
	std::vector<flame::FileHandle::Task> tasks;
};

struct Dependency {
	std::string name;
	size_t min_version;
	size_t max_version;
};

enum class ContextType {
	none,
	asset,
	dependency
};

class Archive : public Data {
	public:
		Archive(Flint& flint, Config config, std::string name);
		~Archive() override {}

		const std::string get_type() const override {
			return "archive";
		}

		void build() override;

		void author(sol::variadic_args args);
		void description(sol::variadic_args args);
		void version(sol::variadic_args args);
		void compression(sol::variadic_args args);
		void key(sol::variadic_args args);
		void path(sol::variadic_args args);
		void task(sol::variadic_args args);
		void requires(sol::variadic_args args);
		// @todo These could also be implemented as a version of version: version(min, max)
		void version_min(sol::variadic_args args);
		void version_max(sol::variadic_args args);
		void script(sol::variadic_args args);

		void asset(sol::variadic_args args);

	private:
		std::string _author;
		std::string _description;
		size_t _version = 1;
		// @todo We need to expose compression to lua: flame.compression.{zlib,none}
		flame::Compression _compression = flame::Compression::zlib;
		std::string _key;

		std::vector<Asset> _assets;
		std::vector<Dependency> _dependencies;

		// @todo This is always size()-1
		size_t _context;
		// This does not default to none
		ContextType _context_type = ContextType::none;

		Flint& _flint;
};
