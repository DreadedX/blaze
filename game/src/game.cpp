#include "asset_list.h"
#include "engine.h"
#include "asset_manager.h"

#include "version.h"

using namespace blaze;

void handle_chat_message(std::shared_ptr<ChatMessage> event) {
	std::cout << "<Dreaded_X> " << event->get_text() << '\n';
}

void handle_missing_dependencies(std::shared_ptr<MissingDependencies> event) {
	std::cout << "Archive '" << event->get_name() << "' is missing the following dependencies:\n";
	for (auto dependency : event->get_missing()) {
		std::cout << dependency.first << ':' << dependency.second << '\n';
	}
}

void lang_test(std::shared_ptr<blaze::Language> lang) {
		std::cout << lang->get("tutorial.part1") << '\n';

		std::cout << lang->get("pickaxe.name") << '\n';
		std::cout << lang->get("pickaxe.description", {47, 100}) << '\n';

}

int main() {
	std::cout << "Version number: " << get_version_number() <<'\n';
	std::cout << "Version string: " << get_version_string() <<'\n';

	// Override LuaTest in archive with version from disk
	{
		// @todo This should go into a lua script
		flame::MetaAsset lua_asset("test/Script", "assets/script/Test.lua", 10);
		flame::asset_list::add(lua_asset);
	}

	// Initialze engine
	// @todo Make it so we do not have to keep a reference around if we just want to register and forget
	auto asdf = event_bus::Subscription<MissingDependencies>(std::ref(handle_missing_dependencies));
	#ifdef ANDROID
		blaze::initialize({"/data/data/nl.mtgames.blazebootstrap/files/base.flm", "/data/data/nl.mtgames.blazebootstrap/files/test.flm"});
	#else
		blaze::initialize({"archives/base.flm", "archives/test.flm"});
	#endif

	// Flame tests
	{
		std::cout << "====ASSETS====\n";
		flame::asset_list::debug_list_meta_assets();
		std::cout << "==============\n";
	}

	// asset_manager
	// auto script = asset_manager::new_asset<Script>("Test");
	auto en = asset_manager::new_asset<Language>("English");
	get_lua_state().set_function("get_lang", [en]{
		return en;
	});
	{
		auto nl = asset_manager::new_asset<Language>("Dutch");

		// Wait for all gameassets to be loaded and show progress
		auto total_count = asset_manager::loading_count();
		auto not_loaded_count = total_count;
		while (not_loaded_count  > 0) {
			std::cout << "Loaded assets: " << total_count-not_loaded_count << '/' << total_count << '\n';
			asset_manager::load_assets();

			// Example of a loading screen
			not_loaded_count = asset_manager::loading_count();
		}
		std::cout << "Loaded assets: " << total_count-not_loaded_count << '/' << total_count << '\n';

		// Simulate the core game loop
		for (int i = 0; i < 3; ++i) {
			update();
			// script->update();
		}
			
		lang_test(en);
		lang_test(nl);
	}

	// Event bus test
	{
		// Lua also registers a handler
		auto sub = event_bus::Subscription<ChatMessage>(std::ref(handle_chat_message));

		event_bus::send(std::make_shared<ChatMessage>("Hello world!"));
		event_bus::send(std::make_shared<ChatMessage>("This is a test"));
	}
}

#ifdef ANDROID
#include <jni.h>
#include <android/log.h>

class androidbuf : public std::streambuf {
	public:
		enum { bufsize = 128 }; // ... or some other suitable buffer size
		androidbuf() { this->setp(buffer, buffer + bufsize - 1); }

	private:
		int overflow(int c)
		{
			if (c == traits_type::eof()) {
				*this->pptr() = traits_type::to_char_type(c);
				this->sbumpc();
			}
			return this->sync()? traits_type::eof(): traits_type::not_eof(c);
		}

		int sync()
		{
			int rc = 0;
			if (this->pbase() != this->pptr()) {
				char writebuf[bufsize+1];
				memcpy(writebuf, this->pbase(), this->pptr() - this->pbase());
				writebuf[this->pptr() - this->pbase()] = '\0';

				rc = __android_log_write(ANDROID_LOG_INFO, "std", writebuf) > 0;
				this->setp(buffer, buffer + bufsize - 1);
			}
			return rc;
		}

		char buffer[bufsize];
};

extern "C" {
	JNIEXPORT void JNICALL Java_nl_mtgames_blazebootstrap_BootstrapActivity_start(JNIEnv* env, jobject thiz) {
		std::cout.rdbuf(new androidbuf);
		main();
	}
}
#endif
