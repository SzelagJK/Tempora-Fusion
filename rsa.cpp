#include "rsa.h"

RSAParams::RSAParams(ZZ N, ZZ phi) : N(std::move(N)), phi(std::move(phi)) {}
const ZZ& RSAParams::getN() const {return N;}
const ZZ& RSAParams::getPhi() const {return phi;}

RSAParams setupRSA(long lambda) {
        const ZZ p = GenPrime_ZZ(lambda/2);
        const ZZ q = GenPrime_ZZ(lambda/2);
        const ZZ N = p*q;
        const ZZ phi = (p-1)*(q-1);
        return RSAParams(N, phi);
}

