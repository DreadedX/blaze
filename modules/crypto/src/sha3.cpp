#include "sha3.h"

#include <bitset>
#include <cmath>

#include <iostream>
#include <cassert>

// @todo Clean everything up and optimize

class State {
	public:
		State(size_t w) : _w(w), _l(log2(w)), _state(25*w) {}

		bool get(size_t x, size_t y, size_t z) const {
			index_check(x,y,z);
			return _state[_w*(5*y+x)+z];
		}
		void set(size_t x, size_t y, size_t z, bool value) {
			index_check(x,y,z);
			_state[_w*(5*y+x)+z] = value;
		}
		void set_state(std::vector<bool> state) {
			assert(state.size() == _state.size());
			_state = state;
		}
		auto get_state() const {
			return _state;
		}
		inline auto get_w() const {
			return _w;
		}
		inline auto get_l() const {
			return _l;
		}

		// Copy constructor
		State(const State& other) : _w(other._w), _l(other._l) {
			_state = other._state;
		}
		// Move constructor
		State(State&& other) : _w(other._w), _l(other._l) {
			_state = std::move(other._state);
		}
		// Move assignment operator
		State& operator=(State&& other) {
			// Make sure that the state are of the same size
			assert(_w == other._w);

			_state = std::move(other._state);

			return *this;
		}

	private:
		const size_t _w;
		const size_t _l;
		std::vector<bool> _state;

		void index_check(size_t x, size_t y, size_t z) const {
			#ifndef NDEBUG
				if (x >= 5 || y >= 5 || z >= _w) { throw std::runtime_error("Index out of bounds"); }
			#endif
		}
};

State theta(State A) {
	size_t w = A.get_w();
	static std::vector<bool> C(5*w);

	for (size_t x = 0; x < 5; ++x) {
		for (size_t z = 0; z < w; ++z) {
			C[w*x + z] = A.get(x, 0, z) ^ A.get(x, 1, z) ^ A.get(x, 2, z) ^ A.get(x, 3, z) ^ A.get(x, 4, z);
		}
	}


	static std::vector<bool> D(5*w);

	for (size_t x = 0; x < 5; ++x) {
		for (size_t z = 0; z < w; ++z) {
			D[w*x + z] = C[w*((x-1+5) % 5) + z] ^ C[w*((x+1) % 5) + ((z-1+w) % w)];
			for (size_t y = 0; y < 5; ++y) {
				A.set(x, y, z, A.get(x, y, z) ^ D[w*x+ z]);
			}
		}
	}

	return A;
}

State rho(State A) {
	size_t w = A.get_w();
	State Aprime = A;

	size_t x = 1;
	size_t y = 0;

	for (size_t t = 0; t < 24; ++t) {
		for (size_t z = 0; z < w; ++z) {
			Aprime.set(x, y, z, A.get(x, y, (z-((t+1)*(t+2))/2) % w));
		}

		auto temp = y;
		y = (2*x + 3*y) % 5;
		x = temp;
	}

	return Aprime;
}

State pi(State A) {
	size_t w = A.get_w();
	State Aprime(w);

	for (size_t y = 0; y < 5; ++y) {
		for (size_t x = 0; x < 5; ++x) {
			for (size_t z = 0; z < w; ++z) {
				Aprime.set(x, y, z, A.get((x+3*y) % 5, x, z));
			}
		}
	}

	return Aprime;
}

State chi(State A) {
	size_t w = A.get_w();
	State Aprime(w);

	for (size_t y = 0; y < 5; ++y) {
		for (size_t x = 0; x < 5; ++x) {
			for (size_t z = 0; z < w; ++z) {
				auto temp = A.get(x, y, z) ^ ((A.get((x+1) % 5, y, z) ^ 1) * A.get((x+2) % 5, y, z));
				Aprime.set(x, y, z, temp);
			}
		}
	}

	return Aprime;
}

bool rc(size_t t) {
	if ((t % 255) == 0) {
		return 1;
	}
	static std::vector<bool> R(9);
	std::fill(R.begin(), R.end(), 0);
	
	R[0] = 1;

	for (size_t i = 1; i < (t % 255)+1; ++i) {
		for (size_t i = 8; i--;) {
			R[i + 1] = R[i];
		}
		R[0] = 0;

		R[0] = R[0] ^ R[8];
		R[4] = R[4] ^ R[8];
		R[5] = R[5] ^ R[8];
		R[6] = R[6] ^ R[8];
	}

	return R[0];
}

State iotta(State A, size_t i_r) {
	size_t w = A.get_w();
	size_t l = A.get_l();

	static std::vector<bool> RC(w);

	for (size_t j = 0; j < l+1; ++j) {
		size_t index;
		if (j == 0) {
			index = 0;
		} else {
			index = (2 << (j-1)) - 1;
		}
		RC[index] = rc(j + 7*i_r);
	}

	for (size_t z = 0; z < w; ++z) {
		A.set(0, 0, z, A.get(0, 0, z) ^ RC[z]);
	}

	return A;
}

State round(State A, size_t i_r) {
	A = theta(std::move(A));
	A = rho(std::move(A));
	A = pi(std::move(A));
	A = chi(std::move(A));
	A = iotta(std::move(A), i_r);

	return A;
}

std::vector<bool> keccakp(std::vector<bool> S, size_t n_r) {
	size_t b = S.size();
	assert(b == 25 || b == 50 || b == 100 || b == 200 || b == 400 || b == 800 || b == 1600);
	size_t w = b/25;

	State A(w);
	A.set_state(S);

	size_t l = A.get_l();

	for (size_t i_r = (12+2*l) - n_r; i_r < 12+2*l; ++i_r) {
		A = round(std::move(A), i_r);
	}


	return A.get_state();
}

std::vector<bool> pad101(size_t x, int32_t m) {
	// @todo Underflow issues
	// size_t j = (-m-2) % x;
	size_t j = x - m - 2;
	std::vector<bool> padding(2+j);
	*padding.begin() = 1;
	*(padding.end()-1) = 1;

	return padding;
}

std::vector<bool> to_bits(std::vector<uint8_t> data) {
	size_t bits_per = sizeof(uint8_t) * 8;
	size_t bitcount = data.size() * bits_per;
	std::vector<bool> bits(bitcount);

	for (size_t i = 0; i < bitcount; ++i) {
		size_t mask = 1 << (i % bits_per);
		bits[i] = data[i/bits_per] & mask;
	}

	return bits;
}

std::vector<uint8_t> from_bits(std::vector<bool> bits) {
	size_t bits_per = sizeof(uint8_t) * 8;
	size_t size = bits.size() / bits_per;
	std::vector<uint8_t> data(size);

	for (size_t i = 0; i < size; ++i) {
		for (size_t j = 0; j < bits_per; ++j) {
			data[i] |= bits[i*bits_per+j] << j;
		}
	}

	return data;
}

namespace CRYPTO_NAMESPACE {
	void SHA3::process() {
		size_t n = _P.size() / _r;
		for (size_t i = 0; i < n; ++i) {
			for (size_t j = 0; j < _r; ++j) {
				_S[j] = _S[j] ^ _P[j + _r*i];
			}

			_S = keccakp(_S, 24);
		}
	}

	void SHA3::update(std::vector<uint8_t> data) {
		std::vector<bool> N = to_bits(data);
		_P.insert(_P.end(), N.begin(), N.end());

		process();

		_P.erase(_P.begin(), _P.begin() + _r*(_P.size()/_r));
	}

	std::vector<uint8_t> SHA3::finalize() {
		_P.push_back(0);
		_P.push_back(1);
		std::vector<bool> padding = pad101(_r, _P.size());
		_P.insert(_P.end(), padding.begin(), padding.end());

		process();

		std::vector<bool> Z;

		while (true) {
			Z.insert(Z.end(), _S.begin(), _S.begin() + _r);
			if (Z.size() > _d) {
				std::vector<bool> Z_final(Z.begin(), Z.begin()+_d);
				assert(Z_final.size() == _d);
				return from_bits(Z_final);
			}
			_S = keccakp(_S, 24);
		}
	}
}
