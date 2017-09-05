#pragma once

#include "asset.h"
#include "archive.h"

#include <unordered_map>

namespace blaze::flame {
	class AssetList {
		public:
			void add_archive(Archive archive) {
				// Use the file stream to load all individual assets into Assets and add them to the list
				auto afs = archive.get_async_fstream();
				auto& fs = afs->lock();

				fs.seekg(0, std::ios::end);
				unsigned long archive_size = fs.tellg();

				unsigned long next_asset = PUBLIC_KEY_SIZE + SIGNATURE_SIZE + sizeof(MAGIC) + archive.get_author().length()+1 +archive.get_description().length()+1;

				while (next_asset < archive_size) {
					fs.seekg(next_asset);
					std::string name;
					binary::read(fs, name);
					uint8_t version;
					binary::read(fs, version);
					uint32_t size;
					binary::read(fs, size);
					uint32_t offset = fs.tellg();

					next_asset = offset + size;

					// Check if we have already 
					auto existing = find_asset(name);
					if (existing) {
						if (existing->get_version() < version) {
							std::cout << "Replacing asset with newer version: " << name << '\n';
						} else if(existing->get_version() > version) {
							std::cout << "Already loaded newer asset: " << name << '\n';
							continue;
						} else {
							std::cerr << __FILE__ << ':' << __LINE__ << ' ' << "Conflicting assets with same version\n";
							// There is no way we can handle this situation, so we just really on load order
							continue;
						}
					}

					_assets[name] = std::make_shared<Asset>(name, afs, version, offset, size);
				}

				afs->unlock();
			}

			const std::shared_ptr<Asset> find_asset(std::string name) const {
				auto asset = _assets.find(name);
				if (asset != _assets.end()) {
						return asset->second;

				}
				return nullptr;
			}

			void debug_list_assets() {
				for (auto& asset : _assets) {
					std::cout << asset.first << '\n';
				}
			}

		private:
			// @todo Are we really going to do yet another layer of pointers
			// @todo A hash map is a better idea for making assets searchable
			std::unordered_map<std::string, std::shared_ptr<Asset>> _assets;
	};
}
