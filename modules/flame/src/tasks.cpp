#include "tasks.h"

// @todo We need to get rid of CryptoPP and just implement it ourselves
// CryptoPP has a header with the same name
#include </usr/include/zlib.h>

#include <iostream>
#include <cassert>

#define LEVEL 9
#define CHUNK_SIZE 1024

namespace FLAME_NAMESPACE::zlib {
	std::vector<uint8_t> compress(std::vector<uint8_t> in) { 
		z_stream stream;
		stream.zalloc = Z_NULL;
		stream.zfree = Z_NULL;
		stream.opaque = Z_NULL;
		stream.avail_in = in.size();
		stream.next_in = in.data();

		int ret = deflateInit(&stream, LEVEL);
		assert(ret == Z_OK /* zlib init failed */);

		int have = 0;
		std::vector<uint8_t> out(0);

		do {
			out.resize(out.size() + CHUNK_SIZE);

			stream.avail_out = CHUNK_SIZE;
			stream.next_out = out.data() + out.size() - CHUNK_SIZE;
			ret = deflate(&stream, Z_FINISH);
			assert(ret != Z_STREAM_ERROR /* deflate failed */);

			have += CHUNK_SIZE - stream.avail_out;
		} while(stream.avail_out == 0);

		out.resize(have);

		// assert(ret == Z_STREAM_END);
		deflateEnd(&stream);

		return out;
	}

	std::vector<uint8_t> decompress(std::vector<uint8_t> in) { 
		z_stream stream;
		stream.zalloc = Z_NULL;
		stream.zfree = Z_NULL;
		stream.opaque = Z_NULL;
		stream.avail_in = in.size();
		stream.next_in = in.data();

		int ret = inflateInit(&stream);
		assert(ret == Z_OK /* zlib init failed */);

		int have = 0;
		std::vector<uint8_t> out(0);

		while (stream.avail_in > 0) {
			out.resize(out.size() + CHUNK_SIZE);

			stream.avail_out = CHUNK_SIZE;
			stream.next_out = out.data() + out.size() - CHUNK_SIZE;
			ret = inflate(&stream, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR /* deflate failed */);

			have += CHUNK_SIZE - stream.avail_out;
		}

		out.resize(have);

		// assert(ret == Z_STREAM_END);
		inflateEnd(&stream);

		return out;
	}
}
