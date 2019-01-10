#include "cvar.h"

namespace BLAZE_NAMESPACE {
	std::unordered_map<std::string, std::variant<int, float>> CVar::_values;
}

