#include "external_task.h"

#include <dlfcn.h>

std::function<std::vector<uint8_t>(std::vector<uint8_t>)> get_external_task(std::string path) {
	void* handle = dlopen(path.c_str(), RTLD_NOW);

	std::function<std::vector<uint8_t>(std::vector<uint8_t>)> (*function)();

	function = (std::function<std::vector<uint8_t>(std::vector<uint8_t>)> (*)())dlsym(handle, "get_task");

	return function();
}
