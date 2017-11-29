#pragma once

#include "blaze.h"

#include <string>

#if __ANDROID__
	#include <android/log.h>
#endif

// @todo Move each of these things into their own module and make the build system link the proper modules
// @todo Print function that calls native implemtation or something along thoses lines
namespace BLAZE_NAMESPACE {

	class Platform {
		public:
			virtual const std::string get_base_path() const = 0;
			virtual bool has_async_support() const = 0;
	};

#if __linux__
	class Linux : public Platform {
		public:
			const std::string get_base_path() const override {
				return "archives/";
			}

			bool has_async_support() const override {
				return true;
			}
	};
#else
	class Linux : public Platform {};
#endif

#if __ANDROID__

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

	class Android : public Platform {
		public:
			Android() {
				std::cout.rdbuf(new androidbuf);
				std::cerr.rdbuf(new androidbuf);
			}

			const std::string get_base_path() const override {
				return "/storage/emulated/0/Android/data/nl.mtgames.blazebootstrap/files/";
			}

			bool has_async_support() const override{
				return true;
			}
	};
#else
	class Android : public Platform {};
#endif

#if __EMSCRIPTEN__
	class Web : public Platform {
		public:
			const std::string get_base_path() const override {
				return "archives/";
			}

			bool has_async_support() const override{
				return false;
			}
	};
#else
	class Web : public Platform {};
#endif
}
