#pragma once

#include "graphics_backend.h"

#include "logger.h"

#include "vulkan/vulkan.hpp"

#include <optional>

#include "game_asset.h"

namespace BLAZE_NAMESPACE {
	class VulkanBackend;

	// Platforms that support vulkan need to implement this
	class VulkanPlatformSupport {
		public:
			virtual void vulkan_init(VulkanBackend* backend) = 0;
			virtual void vulkan_update() = 0;
			virtual void vulkan_destroy() = 0;
			virtual bool vulkan_is_running() = 0;
			virtual void vulkan_get_framebuffer_size(int& width, int& height) = 0;

			virtual VkSurfaceKHR vulkan_create_surface(VkInstance instance) = 0;

			virtual std::vector<const char*> vulkan_get_required_extensions() = 0;
	};

	class VulkanTexture : public GameAsset {
		public:
			VulkanTexture(std::string asset_name, VulkanBackend* backend);

			~VulkanTexture();

			VkImageView _texture_image_view;
			VkSampler _texture_sampler;

		private:
			std::vector<uint8_t> load(std::vector<uint8_t> data);

			void create_texture_image(std::vector<uint8_t>& data);
			void create_texture_image_view();
			void create_texture_sampler();

			VulkanBackend* _backend;

			VkImage _texture_image;
			VkDeviceMemory _texture_image_memory;
	};

	class VulkanShader : public GameAsset {
		public:
			// @todo Split the shader stages into there own assets
			VulkanShader(std::string vertex_name, std::string fragment_name, VulkanBackend* backend);
			~VulkanShader();

			bool is_loaded() override;

			void recreate();

			void commands(VkCommandBuffer command_buffer);

			VkPipelineLayout _pipeline_layout;
			VkPipeline _pipeline;
			VkDescriptorSetLayout _descriptor_set_layout;

		private:
			void create_graphics_pipeline();
			void create_descriptor_set_layout();

			flame::DataHandle _fragment_handle;
			VulkanBackend* _backend;

			VkShaderModule _vert_shader_module;
			VkShaderModule _frag_shader_module;
	};

	class VulkanMaterial {
		public:
			VulkanMaterial(std::shared_ptr<VulkanShader> shader, std::shared_ptr<VulkanTexture> texture, VulkanBackend* backend);

			void commands(VkCommandBuffer command_buffer, int index);

			void recreate() {
				_shader->recreate();
			}

			std::vector<VkDescriptorSet> _descriptor_sets;
		private:
			void create_descriptor_sets();

			std::shared_ptr<VulkanShader> _shader;
			std::shared_ptr<VulkanTexture> _texture;

			VulkanBackend* _backend;
	};

	class VulkanModel : public GameAsset {
		public:
			VulkanModel(std::string asset_name, std::shared_ptr<VulkanMaterial> material, VulkanBackend* backend);
			~VulkanModel();

			void commands(VkCommandBuffer command_buffer, int index);

			void recreate();

			VkBuffer _vertex_buffer;
			VkDeviceMemory _vertex_buffer_memory;

			VkBuffer _index_buffer;
			VkDeviceMemory _index_buffer_memory;

			std::vector<uint32_t> _indices;

		private:
			std::vector<uint8_t> load(std::vector<uint8_t> data);

			void load_model(std::vector<uint8_t>& data);
			void create_vertex_buffer();
			void create_index_buffer();

			std::shared_ptr<VulkanMaterial> _material;
			VulkanBackend* _backend;

			std::vector<Vertex> _vertices;
	};

	class VulkanBackend : public GraphicsBackend {
		public:

			VulkanBackend();

			void init() override;

			void update() override;

			void cleanup() override;

			bool is_running() override;

			bool _framebuffer_resized = false;

			friend class VulkanTexture;
			friend class VulkanModel;
			friend class VulkanShader;
			friend class VulkanMaterial;

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
			void create_framebuffers();
			void create_command_pools();
			void create_depth_resources();
			void create_descriptor_pool();
			void create_uniform_buffers();
			void load_assets();
			void create_command_buffers();
			void create_sync_objects();

			void create_image(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& image_memory);
			void transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);
			void create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags propteries, VkBuffer& buffer, VkDeviceMemory& buffer_memory);
			void copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);
			void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
			VkImageView create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags);

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
			VkFormat find_supported_format(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
			VkFormat find_depth_format();
			bool has_stencil_component(VkFormat format);

			uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags propteries);

			VkCommandBuffer begin_single_time_commands(VkCommandPool command_pool);
			void end_single_time_commands(VkCommandBuffer command_buffer, VkQueue queue);

			#if !defined(__ANDROID__)
				static VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data);

				const std::vector<const char*> _validation_layers = {
					"VK_LAYER_LUNARG_standard_validation"
				};
			#else
				static VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(VkDebugReportFlagsEXT msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char * pLayerPrefix, const char * pMsg, void * pUserData);

				const std::vector<const char*> _validation_layers = {
					"VK_LAYER_LUNARG_core_validation"
				};
			#endif

			const std::vector<const char*> _device_extensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};

			const bool _enable_validation_layers;

			VkInstance _instance;
			#if !defined(__ANDROID__)
				VkDebugUtilsMessengerEXT _callback;
			#else
				VkDebugReportCallbackEXT _callback;
			#endif

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

			VkCommandPool _graphics_command_pool;
			VkCommandPool _transfer_command_pool;

			VkDescriptorPool _descriptor_pool;

			std::shared_ptr<VulkanModel> _vulkan_model;

			std::vector<VkImageView> _swap_chain_image_views;
			std::vector<VkFramebuffer> _swap_chain_framebuffers;
			std::vector<VkCommandBuffer> _graphics_command_buffers;

			VkImage _depth_image;
			VkDeviceMemory _depth_image_memory;
			VkImageView _depth_image_view;

			std::vector<VkSemaphore> _image_available_semaphores;
			std::vector<VkSemaphore> _render_finished_semaphores;
			std::vector<VkFence> _in_flight_fences;

			std::vector<VkBuffer> _uniform_buffers;
			// @todo This can also be one piece if we store the offsets
			std::vector<VkDeviceMemory> _uniform_buffers_memory;

			const int MAX_FRAMES_IN_FLIGHT = 2;
			size_t _current_frame = 0;
	};
}
