#pragma once

#include "platform/platform.h"

#include <jni.h>

#include <android_native_app_glue.h>

#include "graphics_backend/vulkan.h"

extern bool _android_running;
extern ANativeWindow* _android_window;
extern bool _android_resized;

namespace BLAZE_NAMESPACE::platform {
	class Android : public Platform, public VulkanPlatformSupport {
		public:
			const std::string get_base_path() const override;
			bool has_async_support() const override;
			logger::LogHandler get_logger() override;

			void vulkan_init(VulkanBackend* backend) override;
			void vulkan_update() override;
			void vulkan_destroy() override;
			bool vulkan_is_running() override;
			void vulkan_get_framebuffer_size(int& width, int& height) override;
			VkSurfaceKHR vulkan_create_surface(VkInstance instance) override;
			std::vector<const char*> vulkan_get_required_extensions() override;
	};
}
