#include "lang.h"

#include "iohelper/read.h"

std::vector<std::string> split_string(std::string s) {
	std::vector<std::string> split;
	size_t i = 0;
	size_t j = 0;
	while (i != std::string::npos) {
		i = s.find_first_of('.', j);
		split.push_back(s.substr(j, i-j));
		j = i + 1;
	}

	return split;
}

namespace LANG_NAMESPACE {

	Node parse_file(std::istream& in) {
		Node root;
		Node* current_node = &root;

		type current = (type)iohelper::read<uint8_t>(in);
		while (!in.eof()) {
			switch (current) {
				case type::NODE: {
					int level = iohelper::read_length(in);
					std::string name = iohelper::read<std::string>(in);

					while (level <= current_node->level) {
						current_node = current_node->parent;
					}

					auto it = current_node->children.find(name);
					if (it == current_node->children.end()) {
						Node new_node;
						new_node.name = name;
						new_node.parent = current_node;
						current_node->children.emplace(name, std::move(new_node));
					}
					current_node = &current_node->children[name];
					current_node->level = level;

					break;
				}
				case type::KEY: {
					std::string key = iohelper::read<std::string>(in);
					std::string value = iohelper::read<std::string>(in);

					auto it = current_node->children.find(key);
					if (it == current_node->children.end()) {
						Node new_node;
						new_node.name = key;
						new_node.parent = current_node;
						current_node->children.emplace(key, std::move(new_node));
					}
					current_node->children[key].value = value;

					break;
				}
			}
			current = (type)iohelper::read<uint8_t>(in);
		}

		return std::move(root);
	}

	std::string Node::get_value(std::string name) {
		auto split = split_string(name);

		lang::Node* target = this;

		for (auto& s : split) {
			auto it = target->children.find(s);
			if (it == target->children.end()) {
				return "(undefined)";
			}
			target = &it->second;
		}

		return target->value;
	}
}

