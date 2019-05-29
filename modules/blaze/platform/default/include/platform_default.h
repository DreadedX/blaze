#include "platform/platform.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "graphics_backend/vulkan.h"

namespace BLAZE_NAMESPACE::platform {
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

	class Desktop : public Platform, public VulkanGLFW {
		public:
			const std::string get_base_path() const override {
				return "./";
			}

			bool has_async_support() const override {
				return true;
			}

			logger::LogHandler get_logger() override {
				return _internal::std_logger;
			}
		};
}
