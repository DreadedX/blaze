// @note This should be the only file in the projects that has platform dependent preprocessor macros
// Everywhere else we should use constexpr
// Platform specific code should go into platform

#include "engine.h"
#include "platform/platform.h"

#include "version.h"
#include "enviroment.h"

#include <iostream>

// @note The user has to implement this
void game();

// @todo We need to move all of this into some kind of special class thingy
#if ANDROID
	#include <jni.h>
	#include <android/log.h>

	// @todo This has problems if we use '\n' instead of std::endl
	class custombuf : public std::streambuf {
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

					rc = __android_log_write(ANDROID_LOG_INFO, "Native", writebuf) > 0;
					this->setp(buffer, buffer + bufsize - 1);
				}
				return rc;
			}

			char buffer[bufsize];
	};

	extern "C" {
		JNIEXPORT void JNICALL Java_nl_mtgames_blazebootstrap_BootstrapActivity_start(JNIEnv* env, jobject thiz) {
			main();
		}
	}

#else
	class custombuf : public std::streambuf {};
#endif

// This is the entry point of the game engine
int main() {
	// We can put os specific things here
	// @todo In the future when we have a platform class we can set it to the correct one here
	if constexpr (enviroment::os == enviroment::OS::Linux) {
		blaze::set_platform<blaze::Linux>();
	} else if constexpr (enviroment::os == enviroment::OS::Android) {
		blaze::set_platform<blaze::Android>();

		// @todo We need to set this in platform
		std::cout.rdbuf(new custombuf);
		std::cerr.rdbuf(new custombuf);
	} else if constexpr (enviroment::os == enviroment::OS::Web) {
		blaze::set_platform<blaze::Web>();
	}

	std::cout << "BLZNGN Version: " << get_version_number() << '-' << get_version_string() <<'\n';

	blaze::initialize();

	game();
}

