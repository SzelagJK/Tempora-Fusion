#ifndef TLP_H
#define TLP_H

#include <NTL/ZZ.h>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>
#include <cryptopp/filters.h>
#include <string>

#include "rsa.h"
#include "helper_functions.h"

using namespace NTL;
using namespace CryptoPP;

class TLPPuzzle {
	private:
		ZZ N, r, c;
		long T;
	public:
		TLPPuzzle(ZZ N, ZZ r, long T, ZZ c);
		const ZZ& getR() const;
		const long& getT() const;
		const ZZ& getCiphertext() const;
};

// Note for future: When conducting experiments, modify T to delta and max_ss, and switch from string messages to ZZ
TLPPuzzle generatePuzzle(
		const RSAParams &params,
		const std::string &message,
		long T
);

std::string solvePuzzle(const TLPPuzzle &o, const RSAParams params);

#endif
