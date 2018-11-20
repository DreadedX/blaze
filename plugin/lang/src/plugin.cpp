#include "iohelper/write.h"
#include "iohelper/read.h"

#include <fstream>
#include <iostream>

#include "lang.h"

int yyparse();
extern FILE* yyin;

lang::Node& get_root();

void display_children(lang::Node& node) {
	for (int i = 0; i < node.level - 1; ++i) {
		std::cout << '\t';
	}
	if (!node.children.empty() && !node.name.empty()) {
		std::cout << node.name << ":\n";
	}
	for (auto& [na, n] : node.children) {
		if (!n.value.empty()) {
			for (int i = 0; i < node.level; ++i) {
				std::cout << '\t';
			}
			std::cout << n.name << ": " << n.value << '\n';
		}
	}
	for (auto& [na, n] : node.children) {
		display_children(n);
	}
}

void to_file(std::ostream& f, lang::Node& node) {
	// Store the current node
	if (!node.children.empty() && !node.name.empty()) {
		iohelper::write<uint8_t>(f, (uint8_t)lang::type::NODE);
		iohelper::write_length(f, node.level);
		iohelper::write<std::string>(f, node.name);
	}

	// Store all key value pairs
	for (auto& [na, n] : node.children) {
		if (!n.value.empty()) {
			iohelper::write<uint8_t>(f, (uint8_t)lang::type::KEY);
			iohelper::write<std::string>(f, n.name);
			iohelper::write<std::string>(f, n.value);
		}
	}

	// Go trough all child nodes
	for (auto& [na, n] : node.children) {
		to_file(f, n);
	}
}

int main(int argc, const char* argv[]) {
	if (argc != 2) {
		std::cerr << "No file name specified!\n";
		exit(-1);
	}

	std::string filename = argv[1];

	FILE* myfile = fopen(filename.c_str(), "r");

	if (!myfile) {
		std::cerr << "Failed to open file: " << filename << '\n';
		return -1;
	}

	yyin = myfile;

	yyparse();

	std::ofstream output(filename + "pack", std::ios::out | std::ios::trunc);
	if (!output.is_open()) {
		std::cerr << "Failed to open file: " << filename << "pack" << '\n';
		return -1;
	}

	display_children(get_root());
	to_file(output, get_root());
}
