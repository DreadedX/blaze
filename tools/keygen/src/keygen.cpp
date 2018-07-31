#include "logger.h"

#include "parser.h"

#include <fstream>

#include "binary_helper.h"
#include "archive.h"

#include "rsa.h"

int main(int argc, char* argv[]) {
	logger::add(logger::std_logger);

	// @note This is propably going to be the API, and currently it kinda works
	Parser parser(argv[0]);

	parser.add_required("filename", Required());
	parser.add_option("size", Option("-s", "--size"), "Key size in bits");
	// parser.add_option("verbose", Option("-v"), "Print verbose output");

	try {
		parser.parse(argc, argv);

		// @todo We need to validate the commandline arguments
		int size = parser.get_option("size", 1024);
		std::string filename = parser.get_required<std::string>("filename");

		// @todo Give the user feedback
		LOG_M("Generating {} bit key...\n", size);
		auto keys = crypto::generate_rsa_keys(size);
		LOG_M("Key generated!\n");
		// @note We do not have to store the public key as we always use e=65537
		crypto::store(filename, keys.first);
		LOG_M("Key stored in '{}'\n", filename);
	} catch(std::exception& e) {
		LOG_E("{}\n", e.what());
		parser.print_help();
	}
}
