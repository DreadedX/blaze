#include "trusted_key.h"
#include <vector>

std::vector<uint8_t> n = { %s };

crypto::RSA get_trusted_key() {
	return crypto::RSA(n, crypto::default_e());
}
