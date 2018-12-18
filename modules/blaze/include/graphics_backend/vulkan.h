#pragma once

#include "graphics_backend.h"

#include "logger.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <optional>

namespace BLAZE_NAMESPACE {
	class VulkanBackend : public GraphicsBackend {
		public:
			void init() override;

			void update() override;

			void cleanup() override;

			bool is_running() override;

		private:
			struct QueueFamilyIndices {
				std::optional<uint32_t> graphics_family;
				std::optional<uint32_t> present_family;
				std::optional<uint32_t> transfer_family;

				bool is_complete() {
					return graphics_family.has_value() && present_family.has_value() && transfer_family.has_value();
				}
			};

			struct SwapChainSupportDetails {
				VkSurfaceCapabilitiesKHR capabilities;
				std::vector<VkSurfaceFormatKHR> formats;
				std::vector<VkPresentModeKHR> present_modes;
			};

			void init_window();
			void init_vulkan();

			void cleanup_swap_chain();
			void recreate_swapchain();
			void create_instance();
			void setup_debug_callback();
			void create_surface();
			void pick_physical_device();
			void create_logical_device();
			void create_swap_chain();
			void create_image_views();
			void create_render_pass();
			void create_descriptor_set_layout();
			void create_graphics_pipeline();
			void create_framebuffers();
			void create_command_pools();
			void create_vertex_buffer();
			void create_index_buffer();
			void create_uniform_buffers();
			void create_descriptor_pool();
			void create_descriptor_sets();
			void create_command_buffers();
			void create_sync_objects();

			void create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags propteries, VkBuffer& buffer, VkDeviceMemory& buffer_memory);
			void copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);

			void draw_frame();

			void update_uniform_buffer(uint32_t image_index);
			std::vector<const char*> get_required_extensions();
			bool check_extension_support(std::vector<const char*> required_extensions);
			bool check_validation_layer_support();
			int rate_device_suitability(VkPhysicalDevice device);
			QueueFamilyIndices find_queue_families(VkPhysicalDevice device);
			bool check_device_extension_support(VkPhysicalDevice device);
			SwapChainSupportDetails query_swap_chain_support(VkPhysicalDevice device);
			VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR> available_formats);
			VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR> available_present_modes);
			VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);
			VkShaderModule create_shader_module(const std::vector<uint8_t>& code);

			uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags propteries);

			static void glfw_error_callback(int error, const char* description);
			static void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height);
			static VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data);

			GLFWwindow* _window;
			const uint32_t WIDTH = 800;
			const uint32_t HEIGHT = 600;

			const std::vector<const char*> _validation_layers = {
				"VK_LAYER_LUNARG_standard_validation"
			};

			const std::vector<const char*> _device_extensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};

			const std::vector<Vertex> _vertices = {
				{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
				{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
				{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
				{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
			};

			const std::vector<uint16_t> _indices = {
				0, 1, 2, 2, 3, 0
			};

			#ifdef NDEBUG
				const bool _enable_validation_layers = false;
			#else
				const bool _enable_validation_layers = true;
			#endif

			VkInstance _instance;
			VkDebugUtilsMessengerEXT _callback;

			VkPhysicalDevice _physical_device = VK_NULL_HANDLE;
			VkDevice _device;

			VkQueue _graphics_queue;
			VkQueue _present_queue;
			VkQueue _transfer_queue;

			VkSurfaceKHR _surface;

			VkSwapchainKHR _swap_chain;
			std::vector<VkImage> _swap_chain_images;
			VkFormat _swap_chain_image_format;
			VkExtent2D _swap_chain_extent;

			VkRenderPass _render_pass;
			VkDescriptorSetLayout _descriptor_set_layout;
			VkPipelineLayout _pipeline_layout;

			VkPipeline _graphics_pipeline;

			VkCommandPool _graphics_command_pool;
			VkCommandPool _transfer_command_pool;

			VkDescriptorPool _descriptor_pool;
			std::vector<VkDescriptorSet> _descriptor_sets;

			VkBuffer _vertex_buffer;
			VkDeviceMemory _vertex_buffer_memory;
			VkBuffer _index_buffer;
			VkDeviceMemory _index_buffer_memory;

			std::vector<VkImageView> _swap_chain_image_views;
			std::vector<VkFramebuffer> _swap_chain_framebuffers;
			std::vector<VkCommandBuffer> _graphics_command_buffers;

			std::vector<VkSemaphore> _image_available_semaphores;
			std::vector<VkSemaphore> _render_finished_semaphores;
			std::vector<VkFence> _in_flight_fences;

			std::vector<VkBuffer> _uniform_buffers;
			// @todo This can also be one piece if we store the offsets
			std::vector<VkDeviceMemory> _uniform_buffers_memory;

			const int MAX_FRAMES_IN_FLIGHT = 2;
			size_t _current_frame = 0;

			bool _framebuffer_resized = false;
	};
}
