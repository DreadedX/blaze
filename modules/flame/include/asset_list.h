#pragma once

#include "asset.h"
#include "archive.h"
#include "async_data.h"

#include <unordered_map>

namespace blaze::flame {
	class AssetList {
		public:
			ASyncData find_asset(std::string name) {
				auto asset = _assets.find(name);
				if (asset != _assets.end()) {
						return asset->second.get_data();
				}
				std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "Can not find asset\n";
				return ASyncData();
			}

			void add(Archive& archive) {
				if (archive.is_valid()) {
					_archives.push_back(archive);
				} else {
					std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "Invalid archive\n";
				}
			}
			bool check_dependency(std::pair<std::string, uint16_t> dependency) {
				for (auto &archive : _archives) {
					if (dependency.first == archive.get_name() && dependency.second == archive.get_version()) {
						return true;
					}
				}
				return false;
			}
			void load() {
				for (auto& archive : _archives) {
					if (!archive.is_valid()) {
						std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "Archive invalid\n";
						continue;
					}
					// Check if the dependecies are loaded
					// @todo This needs testing
					bool found = true;
					for (auto& dependency : archive.get_dependencies()) {
						found = check_dependency(dependency);
						if (!found) {
							std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "Missing dependency: " << dependency.first << ':' << dependency.second << '\n';
						}
						break;
					}
					if (!found) {
						continue;
					}

					// Use the file stream to load all individual assets into Assets and add them to the list
					auto afs = archive.get_async_fstream();
					auto& fs = afs->lock();

					fs.seekg(0, std::ios::end);
					unsigned long archive_size = fs.tellg();

					unsigned long next_asset = PUBLIC_KEY_SIZE + SIGNATURE_SIZE + sizeof(MAGIC) + archive.get_name().length()+1 + archive.get_author().length()+1 +archive.get_description().length()+1 + sizeof(uint16_t) /* version */;

					for (auto& dependency : archive.get_dependencies()) {
						next_asset += dependency.first.length()+1 + sizeof(uint16_t);
					}
					// null terminator at the end of the dependency list
					next_asset += 1;

					while (next_asset < archive_size) {
						fs.seekg(next_asset);
						std::string name;
						binary::read(fs, name);
						uint16_t version;
						binary::read(fs, version);
						uint32_t size;
						binary::read(fs, size);
						uint32_t offset = fs.tellg();

						next_asset = offset + size;

						// Check if we have already 
						auto existing = _assets.find(name);
						if (existing != _assets.end()) {
							if (existing->second.get_version() < version) {
								std::cout << "Replacing asset with newer version: " << name << '\n';
							} else if(existing->second.get_version() > version) {
								std::cout << "Already loaded newer asset: " << name << '\n';
								continue;
							} else {
								std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "Conflicting assets with same version\n";
								// There is no way we can handle this situation, so we just really on load order
								continue;
							}
						}

						Asset asset(name, afs, version, offset, size);
						_assets[name] = asset;
					}

					afs->unlock();
				}
			}

			void debug_list_assets() {
				for (auto& asset : _assets) {
					std::cout << asset.first << '\n';
				}
			}

		private:
			std::vector<Archive> _archives;
			std::unordered_map<std::string, Asset> _assets;
	};
}
