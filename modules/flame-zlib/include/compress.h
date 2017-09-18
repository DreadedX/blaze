#pragma once

#include "asset.h"

#include <iostream>
#include <cstring>
#include <cassert>

// @todo We need to get rid of CryptoPP and just implement it ourselves
// CryptoPP has a header with the same name
#include </usr/include/zlib.h>

#define LEVEL 9
#define CHUNK_SIZE 1024

namespace blaze::flame::zlib {
	std::pair<std::unique_ptr<uint8_t[]>, uint32_t> compress(std::pair<std::unique_ptr<uint8_t[]>, uint32_t> info) { 
		z_stream stream;
		stream.zalloc = Z_NULL;
		stream.zfree = Z_NULL;
		stream.opaque = Z_NULL;

		int ret = deflateInit(&stream, LEVEL);

		assert(ret == Z_OK /* zlib init failed */);

		stream.avail_in = info.second;
		stream.next_in = info.first.get();

		// Assuming that the data in will be smaller than the data out
		stream.avail_out = CHUNK_SIZE;
		auto out = std::make_unique<uint8_t[]>(CHUNK_SIZE + sizeof(uint32_t));
		stream.next_out = out.get() + sizeof(uint32_t);

		int flush = Z_FINISH;
		ret = deflate(&stream, flush);

		assert(ret != Z_STREAM_ERROR /* deflate failed */);
		int have = CHUNK_SIZE - stream.avail_out;

		// Write the size marker
		for (size_t i = 0; i < sizeof(uint32_t); ++i) {
			uint8_t byte = ((have >> i*8) & 0xff);
			out[i] = byte;
		}

		// Each compressed segment will be stored in CHUNK_SIZE bytes, that way we can easily read each block
		return std::make_pair(std::move(out), have + sizeof(uint32_t));
	}
}

#undef CHUNK_SIZE
