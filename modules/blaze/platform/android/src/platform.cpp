#include "platform_android.h"
#include "engine.h"

#include "cvar.h"

#include <android/log.h>

#include <iostream>
#include <thread>

int main(int argc, const char* argv[]);

bool _android_running = false;
ANativeWindow* _android_window = nullptr;
blaze::VulkanBackend* _vulkan_backed = nullptr;

std::thread game_thread;

void handle_cmd(android_app* app, int32_t cmd) {
	switch (cmd) {
		case APP_CMD_INIT_WINDOW:
			__android_log_print(ANDROID_LOG_INFO, "BlazeNativeHandler", "Init");
			_android_window = app->window;
			_android_running = true;
			game_thread = std::thread(main, 0, nullptr);
			break;

		case APP_CMD_TERM_WINDOW:
			__android_log_print(ANDROID_LOG_INFO, "BlazeNativeHandler", "Terminate");
			_android_running = false;
	 		break;

		case APP_CMD_CONFIG_CHANGED:
			__android_log_print(ANDROID_LOG_INFO, "BlazeNativeHandler", "Config change");
			// @todo This is super janky
			_vulkan_backed->_framebuffer_resized = true;
			break;

		default:
			//LOG_D("Event not handled: {}\n", cmd);
			__android_log_print(ANDROID_LOG_INFO, "BlazeNativeHandler", "Unhandled");
	}
}

void android_main(struct android_app* app) {
	__android_log_print(ANDROID_LOG_INFO, "BlazeNative", "Starting");
	app->onAppCmd = handle_cmd;

	int events;
	android_poll_source* source;

	do {
		if (ALooper_pollAll(_android_running ? 1 : 0, nullptr, &events, (void**)&source) >= 0) {
			if (source != nullptr) {
				source->process(app, source);
			}
		}
	} while (app->destroyRequested == 0);

	game_thread.join();
}

namespace BLAZE_NAMESPACE::platform {

	void set() {
		blaze::set_platform<Android>();
	}

	const std::string Android::get_base_path() const {
		// @todo We need to just use the context to get the actual path
		// return "/data/user/0/nl.mtgames.blaze/files/";
		return "/storage/emulated/0/Android/data/nl.mtgames.blaze/files/";
	}

	bool Android::has_async_support() const {
		return true;
	}

	logger::LogHandler Android::get_logger() {
		return [](Level level, std::string, int, std::string message){
			static int& log_level = CVar::get<int>("log_level");

			if (level < (Level)log_level) {
				return;
			}

			switch (level) {
				case Level::debug:
					__android_log_print(ANDROID_LOG_DEBUG, "BlazeNative", message.c_str());
					break;

				case Level::message:
					__android_log_print(ANDROID_LOG_INFO, "BlazeNative", message.c_str());
					break;

				case Level::error:
					__android_log_print(ANDROID_LOG_ERROR, "BlazeNative", message.c_str());
					break;

			}
		};
	}

	void Android::vulkan_init(VulkanBackend* backend) {
		_vulkan_backed = backend;
	}

	void Android::vulkan_update() {

	}

	void Android::vulkan_destroy() {

	}

	bool Android::vulkan_is_running() {
		return _android_running;
	}

	void Android::vulkan_get_framebuffer_size(int& width, int& height) {
		width = ANativeWindow_getWidth(_android_window);
		height = ANativeWindow_getHeight(_android_window);

		LOG_D("WIDTH: {}\n", width);
		LOG_D("HEIGHT: {}\n", height);
	}

	VkSurfaceKHR Android::vulkan_create_surface(VkInstance instance) {
		VkAndroidSurfaceCreateInfoKHR create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
		create_info.pNext = nullptr;
		create_info.flags = 0;
		create_info.window = _android_window;

		VkSurfaceKHR surface;
		VkResult result = vkCreateAndroidSurfaceKHR(instance, &create_info, nullptr, &surface);
		LOG_D("result = {}\n", result);
		if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface");
		}

		return surface;
	}

	std::vector<const char*> Android::vulkan_get_required_extensions() {
		std::vector<const char*> extensions;

		extensions.push_back("VK_KHR_surface");
		extensions.push_back("VK_KHR_android_surface");

		return extensions;
	}
}

