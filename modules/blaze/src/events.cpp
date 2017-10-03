#include "events.h"

namespace BLAZE_NAMESPACE {
	std::unordered_map<uint32_t, std::list<std::function<void(std::shared_ptr<Event>)>>> event_bus::_private::subscribers;
}
