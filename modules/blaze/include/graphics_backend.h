#pragma once

#include "blaze.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

namespace BLAZE_NAMESPACE {
	struct Vertex {
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 tex_coord;
	};

	struct UniformBufferObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	class GraphicsBackend {
		public:
			virtual ~GraphicsBackend() {}

			virtual void init() = 0;
			virtual void update() = 0;
			virtual void cleanup() = 0;

			virtual bool is_running() = 0;

	};

	class DummyBackend : public GraphicsBackend{
		public:
			virtual ~DummyBackend() {}

			void init() override {}
			void update() override {}
			void cleanup() override {}

			bool is_running() override {
				return false;
			}
	};
}

