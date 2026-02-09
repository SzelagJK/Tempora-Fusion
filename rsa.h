#ifndef RSA_H
#define RSA_H

#include <NTL/ZZ.h>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>
#include <cryptopp/filters.h>
#include <vector>
#include <cstdint>

using namespace NTL;
using namespace CryptoPP;

// immutable params
class RSAParams {
	private:
   		const ZZ N, phi;

	public:
    		RSAParams(ZZ N, ZZ phi);
    		const ZZ& getN() const;
    		const ZZ& getPhi() const;
};

RSAParams setupRSA(long lambda);

std::vector<RSAParams> generateParamsSet(std::size_t n);

#endif

