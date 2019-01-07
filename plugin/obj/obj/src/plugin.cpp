#include "flint.h"

#include "iohelper/write.h"
#include "iohelper/memstream.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <unordered_map>

#if _WIN32
	#define FLINT_PLUGIN __declspec(dllexport) __stdcall
#else
	#define FLINT_PLUGIN
#endif

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 tex_coord;

	bool operator==(const Vertex& o) const {
		return pos == o.pos && color == o.color && tex_coord == o.tex_coord;
	}
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.tex_coord) << 1);
		}
	};
}

// @todo We should not store everything in global variables
// or we should atleast clear them
extern "C" void FLINT_PLUGIN init(Flint& flint) {
	static bool a = false;
	if (!a) {
		logger::add(logger::std_logger);
		a = true;
	}

	sol::table helper = flint.test_get_lua().create_named_table("obj");
	helper.set_function("load", [] (std::vector<uint8_t> in) {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn;
		std::string err;

		std::vector<Vertex> vertices;
		std::unordered_map<Vertex, uint32_t> unique_vertices = {};
		std::vector<uint32_t> indices;

		iohelper::imemstream istream(in);

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, &istream)) {
			throw std::runtime_error(warn + err);
		}

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex = {};

				vertex.pos = {attrib.vertices[3 * index.vertex_index + 0], attrib.vertices[3 * index.vertex_index + 1], attrib.vertices[3 * index.vertex_index + 2]};

				vertex.color = {1.0f, 1.0f, 1.0f};

				vertex.tex_coord = {attrib.texcoords[2 * index.texcoord_index + 0], 1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

				if (unique_vertices.count(vertex) == 0) {
					unique_vertices[vertex] = vertices.size();
					vertices.push_back(vertex);
				}

				indices.push_back(unique_vertices[vertex]);
			}
		}

		// Try to reserve space
		std::vector<uint8_t> out(vertices.size()*4*8 + indices.size()*4 + 2);
		iohelper::omemstream stream(out);

		iohelper::write_length(stream, vertices.size());
		for (const auto& vertex : vertices) {
			iohelper::write<float>(stream, vertex.pos.x);
			iohelper::write<float>(stream, vertex.pos.y);
			iohelper::write<float>(stream, vertex.pos.z);

			iohelper::write<float>(stream, vertex.color.r);
			iohelper::write<float>(stream, vertex.color.g);
			iohelper::write<float>(stream, vertex.color.b);

			iohelper::write<float>(stream, vertex.tex_coord.x);
			iohelper::write<float>(stream, vertex.tex_coord.y);
		}

		iohelper::write_length(stream, indices.size());
		for (const auto& index : indices) {
			iohelper::write<uint32_t>(stream, index);
		}

		return out;
	});
}
