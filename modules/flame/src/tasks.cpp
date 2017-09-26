#include "tasks.h"

// @todo We need to get rid of CryptoPP and just implement it ourselves
// CryptoPP has a header with the same name
#include </usr/include/zlib.h>

#include <iostream>
#include <cassert>

#define LEVEL 9
#define CHUNK_SIZE 1024

namespace FLAME_NAMESPACE::zlib {
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
		auto temp = std::make_unique<uint8_t[]>(CHUNK_SIZE);
		int have = 0;

		std::unique_ptr<uint8_t[]> data = nullptr;

		do {
			stream.avail_out = CHUNK_SIZE;
			stream.next_out = temp.get();
			ret = deflate(&stream, Z_FINISH);
			assert(ret != Z_STREAM_ERROR /* deflate failed */);

			auto current_data = std::move(data);
			data = std::make_unique<uint8_t[]>(have + CHUNK_SIZE - stream.avail_out);
			if (current_data) {
				memcpy(data.get(), current_data.get(), have);
			}
			memcpy(data.get() + have, temp.get(), CHUNK_SIZE - stream.avail_out);

			have += CHUNK_SIZE - stream.avail_out;
			std::cout << "Left: " << stream.avail_in << " Have: " << have << " Total: " << info.second << '\n';
		} while(stream.avail_out == 0);

		// assert(ret == Z_STREAM_END);
		deflateEnd(&stream);

		return std::make_pair(std::move(data), have);
	}

	std::pair<std::unique_ptr<uint8_t[]>, uint32_t> decompress(std::pair<std::unique_ptr<uint8_t[]>, uint32_t> info) { 
		z_stream stream;
		stream.zalloc = Z_NULL;
		stream.zfree = Z_NULL;
		stream.opaque = Z_NULL;
		stream.avail_in = info.second;
		stream.next_in = info.first.get();

		int ret = inflateInit(&stream);
		assert(ret == Z_OK /* zlib init failed */);

		auto temp = std::make_unique<uint8_t[]>(CHUNK_SIZE);
		int have = 0;

		std::unique_ptr<uint8_t[]> data = nullptr;

		while (stream.avail_in > 0) {
			stream.avail_out = CHUNK_SIZE;
			stream.next_out = temp.get();
			ret = inflate(&stream, Z_NO_FLUSH);
			assert(ret != Z_STREAM_ERROR /* deflate failed */);

			auto current_data = std::move(data);
			data = std::make_unique<uint8_t[]>(have + CHUNK_SIZE - stream.avail_out);
			if (current_data) {
				memcpy(data.get(), current_data.get(), have);
			}
			memcpy(data.get() + have, temp.get(), CHUNK_SIZE - stream.avail_out);

			have += CHUNK_SIZE - stream.avail_out;
			std::cout << "Left: " << stream.avail_in << " Have: " << have << " Total: " << info.second << '\n';
		}

		// assert(ret == Z_STREAM_END);
		inflateEnd(&stream);

		return std::make_pair(std::move(data), have);
	}
}
