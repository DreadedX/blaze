#include "flint.h"

#include "iohelper/write.h"
#include "iohelper/memstream.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#if _WIN32
	#define FLINT_PLUGIN __declspec(dllexport) __stdcall
#else
	#define FLINT_PLUGIN
#endif

// @todo We should not store everything in global variables
// or we should atleast clear them
extern "C" void FLINT_PLUGIN init(Flint& flint) {
	static bool a = false;
	if (!a) {
		logger::add(logger::std_logger);
		a = true;
	}

	sol::table helper = flint.test_get_lua().create_named_table("image");
	helper.set_function("load", [] (std::vector<uint8_t> in) {
		int width;
		int height;
		int channels;

		stbi_uc* pixels = stbi_load_from_memory(in.data(), in.size(), &width, &height, &channels, STBI_rgb_alpha);

		if (!pixels) {
			throw std::runtime_error("Failed to load image!");
		}

		std::vector<uint8_t> output;
		iohelper::omemstream stream(output);

		iohelper::write_length(stream, width);
		LOG_D("Width: {}\n", width);
		iohelper::write_length(stream, height);
		LOG_D("Height: {}\n", height);
		iohelper::write<uint8_t>(stream, STBI_rgb_alpha);
		LOG_D("Channels: {}\n", STBI_rgb_alpha);

		int size = width * height * STBI_rgb_alpha;
		int offset = output.size();
		output.resize(offset + size);

		memcpy(output.data() + offset, pixels, size);
		stbi_image_free(pixels);

		return output;
	});
}
