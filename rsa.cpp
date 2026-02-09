#include "rsa.h"

RSAParams::RSAParams(ZZ N, ZZ phi) : N(std::move(N)), phi(std::move(phi)) {}
const ZZ& RSAParams::getN() const {return N;}
const ZZ& RSAParams::getPhi() const {return phi;}

RSAParams setupRSA(long lambda) {
        const ZZ p = GenPrime_ZZ(lambda);
        const ZZ q = GenPrime_ZZ(lambda);
        const ZZ N = p*q;
        const ZZ phi = (p-1)*(q-1);
        return RSAParams(N, phi);
}

std::vector<RSAParams> generateParamsSet(std::size_t n, long lambda) {
	std::vector<RSAParams> paramsSet;
	paramsSet.reserve(n);
	for (std::size_t i = 0; i < n; i++) {
		RSAParams c_params = setupRSA(lambda);
		paramsSet.push_back(std::move(c_params));
	}
	return paramsSet;
}


