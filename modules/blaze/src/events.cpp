#include "events.h"

namespace BLAZE_NAMESPACE {
	std::unordered_map<uint32_t, std::list<std::function<void(std::shared_ptr<Event>)>>> event_bus::_subscribers;
	uint32_t event_bus::_id_counter;


	ChatMessage::ChatMessage(std::string text) : _text(text) {}

	const std::string& ChatMessage::get_text() const {
		return _text;
	}


	MissingDependencies::MissingDependencies(std::string name, std::vector<std::pair<std::string, uint16_t>> missing) : _name(name), _missing(missing) {}

	const std::vector<std::pair<std::string, uint16_t>>& MissingDependencies::get_missing() const {
		return _missing;
	}

	const std::string& MissingDependencies::get_name() const {
		return _name;
	}


	Error::Error(std::string error, std::string file, size_t line) : _error(error), _context(file + ':' + std::to_string(line)) {}

	const std::string& Error::get_error() const {
		return _error;
	}

	const std::string& Error::get_context() const {
		return _context;
	}
}
