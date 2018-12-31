#include "game_asset/language.h"

#include "iohelper/memstream.h"

#include "fmt/ostream.h"

namespace BLAZE_NAMESPACE {
	Language::Language(std::string asset_name) : GameAsset(asset_name, {std::bind(&Language::load, this, std::placeholders::_1)}) {}

	// Kind of abuse the flame task system to run callbacks on other threads
	std::vector<uint8_t> Language::load(std::vector<uint8_t> data) {
		// LOG_D("Thread id: {} ({})\n", std::this_thread::get_id(), get_name());

		iohelper::imemstream memstream(data);
		_root = lang::parse_file(memstream);

		return data;
	}
}
