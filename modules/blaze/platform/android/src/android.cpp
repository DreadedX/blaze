#include "android.h"

#include <jni.h>
#include <android/log.h>

#include <iostream>

int main();

extern "C" {
	JNIEXPORT void JNICALL Java_nl_mtgames_blazebootstrap_BootstrapActivity_start(JNIEnv* env, jclass clazz) {
		main();
	}
}

// @todo This has problems if we use '\n' instead of std::endl
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

				rc = __android_log_write(ANDROID_LOG_INFO, "Native", writebuf) > 0;
				this->setp(buffer, buffer + bufsize - 1);
			}
			return rc;
		}

		char buffer[bufsize];
};

namespace BLAZE_NAMESPACE::platform {
	Android::Android() {
		std::cout.rdbuf(new androidbuf);
		std::cerr.rdbuf(new androidbuf);
	}

	const std::string Android::get_base_path() const {
		return "/storage/emulated/0/Android/data/nl.mtgames.blazebootstrap/files/";
	}

	bool Android::has_async_support() const {
		return true;
	}
}

