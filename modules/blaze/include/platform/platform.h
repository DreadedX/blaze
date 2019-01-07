#pragma once

#include "blaze.h"

#include "logger.h"

#include <string>
#include <functional>

#if __EMSCRIPTEN__
	#include <emscripten.h>
#endif

#if (!defined(__ANDROID__) && defined(__linux__)) || defined(_WIN32)
	#define GLFW_INCLUDE_VULKAN
	#include "GLFW/glfw3.h"
#endif
#include "graphics_backend/vulkan.h"

// @todo Move each of these things into their own module and make the build system link the proper modules
// @todo Print function that calls native implemtation or something along thoses lines
// @todo Move the environment stuff in here

namespace BLAZE_NAMESPACE::platform {
	class Platform {
		public:
			Platform() {}
			virtual ~Platform() {}

			virtual const std::string get_base_path() const = 0;
			virtual bool has_async_support() const = 0;
			// @todo Give this a better name
			virtual logger::LogHandler get_logger() =0;
	};

#if (!defined(__ANDROID__) && defined(__linux__)) || defined(_WIN32)
	class VulkanGLFW : public VulkanPlatformSupport {
		public:
			void vulkan_init(VulkanBackend* backend) {
				glfwInit();
				glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
				// glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

				glfwSetErrorCallback(glfw_error_callback);

				if (glfwVulkanSupported() != GLFW_TRUE) {
					throw std::runtime_error("Vulkan not available!");
				}

				_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

				glfwSetWindowUserPointer(_window, backend);
				glfwSetFramebufferSizeCallback(_window, vulkan_glfw_framebuffer_size_callback);
			}

			void vulkan_update() override {
				glfwPollEvents();
			}

			void vulkan_destroy() override {
				glfwDestroyWindow(_window);
				_window = nullptr;

				glfwTerminate();
			}

			bool vulkan_is_running() override {
				int v = !glfwWindowShouldClose(_window);
				if (!v) {
					LOG_D("State? {}\n", v);
				}
				return v;
			}

			void vulkan_get_framebuffer_size(int& width, int& height) override {
				glfwGetFramebufferSize(_window, &width, &height);
			}

			VkSurfaceKHR vulkan_create_surface(VkInstance instance) override {
				VkSurfaceKHR surface;
				if (glfwCreateWindowSurface(instance, _window, nullptr, &surface) != VK_SUCCESS) {
					throw std::runtime_error("Failed to create window surface");
				}

				return surface;
			}

			std::vector<const char*> vulkan_get_required_extensions() override {
				uint32_t glfw_extension_count = 0;
				const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

				std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

				return extensions;
			}

		private:
			GLFWwindow* _window = nullptr;

			const uint32_t WIDTH = 800;
			const uint32_t HEIGHT = 600;

			static void glfw_error_callback(int /* error */, const char* description) {
				throw std::runtime_error(description);
			}

			static void vulkan_glfw_framebuffer_size_callback(GLFWwindow* window, int /* width */, int /* height */) {
				auto backend = reinterpret_cast<VulkanBackend*>(glfwGetWindowUserPointer(window));
				backend->_framebuffer_resized = true;
			}
	};
#endif

#if defined(__linux__) && !defined(__ANDROID__)
	class Linux : public Platform, public VulkanGLFW {
		public:
			const std::string get_base_path() const override {
				return "./";
			}

			bool has_async_support() const override {
				return true;
			}

			logger::LogHandler get_logger() override {
				return logger::std_logger;
			}
	};
#else
	class Linux : public Platform {};
#endif

#ifdef _WIN32
	class Windows : public Platform, public VulkanGLFW {
		public:
			const std::string get_base_path() const override {
				return "./";
			}

			bool has_async_support() const override {
				return true;
			}

			logger::LogHandler get_logger() override {
				return logger::std_logger;
			}
	};
#else
	class Windows : public Platform {};
#endif

#ifdef __EMSCRIPTEN__
	class Web : public Platform {
		public:
			Web();

			const std::string get_base_path() const override {
				return "/data/";
			}

			bool has_async_support() const override{
				return false;
			}

			logger::LogHandler get_logger() override {
				return logger::std_logger;
			}
	};
#else
	class Web : public Platform {};
#endif
}

#ifdef __ANDROID__
	#include "android.h"
#else
namespace BLAZE_NAMESPACE::platform {
	class Android : public Platform {};
}
#endif
