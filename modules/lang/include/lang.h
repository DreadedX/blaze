#pragma once

#define LANG_NAMESPACE lang

#include <string>
#include <map>
#include <istream>

namespace LANG_NAMESPACE {
	// @todo Move this together with part of the plugin into a seperate module
	enum class type : uint8_t {
		NODE,
		KEY
	};

	class Node {
		public:
			std::string get_value(std::string name);

			std::string name;
			std::string value;
			std::map<std::string, Node> children;

			int level = 0;

			Node* parent = nullptr;
	};

	Node parse_file(std::istream& in);
}
