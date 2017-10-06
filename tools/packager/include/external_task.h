#pragma once

#include <string>
#include <functional>

std::function<std::vector<uint8_t>(std::vector<uint8_t>)> get_external_task(std::string);

#define EXPORT_TASK(task_name) extern "C" { std::function<std::vector<uint8_t>(std::vector<uint8_t>)> get_task() { return task_name; } }
