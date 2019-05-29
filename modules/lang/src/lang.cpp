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
					std::string str = iohelper::read<std::string>(in);

					std::unordered_map<std::string, std::pair<size_t, Expression>> map;
					size_t map_length = iohelper::read_length(in);
					for (size_t i = 0; i < map_length; ++i) {
						std::string name = iohelper::read<std::string>(in);
						size_t pos = iohelper::read_length(in);
						size_t op_count = iohelper::read_length(in);
						std::vector<std::tuple<Expression::OP, int32_t, std::string>> ops(op_count);
						for (size_t i = 0; i < op_count; ++i) {
							Expression::OP op = (Expression::OP)iohelper::read<uint8_t>(in);
							int32_t criteria = iohelper::read<int32_t>(in);
							std::string str = iohelper::read<std::string>(in);
							ops.emplace_back(op, criteria, str);
						}
						// @todo Parse expressions properly
						if (op_count == 0) {
							map[name] = std::make_pair(pos, Expression());
						} else {
							map[name] = std::make_pair(pos, Expression(ops));
						}
					}

					auto it = current_node->children.find(key);
					if (it == current_node->children.end()) {
						Node new_node;
						new_node.name = key;
						new_node.parent = current_node;
						current_node->children.emplace(key, std::move(new_node));
					}
					current_node->children[key].value = Value(str, map);

					break;
				}
			}
			current = (type)iohelper::read<uint8_t>(in);
		}

		return std::move(root);
	}

	Value Node::get_value(std::string name) {
		auto split = split_string(name);

		lang::Node* target = this;

		for (auto& s : split) {
			auto it = target->children.find(s);
			if (it == target->children.end()) {
				return Value("(undefined)", {});
			}
			target = &it->second;
		}

		return target->value;
	}
}

