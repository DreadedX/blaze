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
		if (!n.value.get_string().empty()) {
			for (int i = 0; i < node.level; ++i) {
				std::cout << '\t';
			}
			std::cout << n.name << ": " << n.value.get_string() << '\n';
			for (auto& [name, pos] : n.value.get_map()) {
				for (int i = 0; i < node.level; ++i) {
					std::cout << '\t';
				}
				std::cout << name << '\n';
			}
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
		if (!n.value.get_string().empty()) {
			iohelper::write<uint8_t>(f, (uint8_t)lang::type::KEY);
			iohelper::write<std::string>(f, n.name);
			iohelper::write<std::string>(f, n.value.get_string());

			auto map = n.value.get_map();
			iohelper::write_length(f, map.size());
			for (auto &[name, p] : map) {
				iohelper::write<std::string>(f, name);
				iohelper::write_length(f, p.first);

				auto ops = p.second.get_ops();
				iohelper::write_length(f, ops.size());
				for (auto& [o, c, s] : ops)  {
					iohelper::write<uint8_t>(f, o);
					iohelper::write<int32_t>(f, c);
					iohelper::write<std::string>(f, s);
				}
			}
		}
	}

	// Go trough all child nodes
	for (auto& [na, n] : node.children) {
		to_file(f, n);
	}
}

std::vector<uint8_t> data;
size_t offset;

int read_input(char* buffer, int* num_bytes_read, int max_bytes_to_read) {
	size_t num_bytes_to_read = max_bytes_to_read;
	size_t num_bytes_remaining = data.size() - offset;

	if (num_bytes_to_read > num_bytes_remaining) {
		num_bytes_to_read = num_bytes_remaining;
	}

	for (size_t i = 0; i < num_bytes_to_read; ++i) {
		buffer[i] = data[offset+i];
	}
	*num_bytes_read = num_bytes_to_read;
	offset += num_bytes_to_read;
	return 0;
}

// int main(int argc, const char* argv[]) {
// 	if (argc != 2) {
// 		std::cerr << "No file name specified!\n";
// 		exit(-1);
// 	}
//
// 	std::string filename = argv[1];
//
// 	FILE* myfile = fopen(filename.c_str(), "r");
//
// 	if (!myfile) {
// 		std::cerr << "Failed to open file: " << filename << '\n';
// 		return -1;
// 	}
//
// 	yyin = myfile;
//
// 	yyparse();
//
// 	std::ofstream output(filename + "pack", std::ios::out | std::ios::trunc);
// 	if (!output.is_open()) {
// 		std::cerr << "Failed to open file: " << filename << "pack" << '\n';
// 		return -1;
// 	}
//
// 	display_children(get_root());
// 	to_file(output, get_root());
// }

#include "flint.h"

#include "iohelper/memstream.h"

#if _WIN32
	#define FLINT_PLUGIN __declspec(dllexport) __stdcall
#else
	#define FLINT_PLUGIN
#endif

// @todo We should not store everything in global variables
// or we should atleast clear them
extern "C" void FLINT_PLUGIN init(Flint& flint) {
	sol::table helper = flint.test_get_lua().create_named_table("langpack");
	helper.set_function("parser", [] (std::vector<uint8_t> in) {
			data = in;
			offset = 0;
			yyin = nullptr;
			yyparse();
			display_children(get_root());

			std::vector<uint8_t> out;
			iohelper::omemstream out_stream(out);
			to_file(out_stream, get_root());

			return out;
	});
}
