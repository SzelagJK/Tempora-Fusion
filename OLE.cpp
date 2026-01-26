#include <NTL/ZZ_p.h>
#include <NTL/ZZ.h>
#include <cryptopp/osrng.h>

using namespace NTL;

// Initialise a k-bit prime field
void InitField(long security_bits = 128) {
    ZZ prime;
    GenPrime(prime, security_bits);
    ZZ_p::init(prime);
}

ZZ_p RandomFieldElement() {
    CryptoPP::AutoSeededRandomPool rng;
    long n = NumBytes(ZZ_p::modulus());
    std::vector<byte> buf(n);
    rng.GenerateBlock(buf.data(), n);

    ZZ z;
    ZZFromBytes(z, buf.data(), n);
    return to_ZZ_p(z);
}


struct OLE_Output {
    ZZ_p value;
};

OLE_Output OLE_Send(const ZZ_p& a, const ZZ_p& b, const ZZ_p& x) {
    OLE_Output out;
    out.value = a * x + b;
    return out;
}

ZZ_p EnhancedOLE(
    const ZZ_p& a,
    const ZZ_p& b,
    const ZZ_p& x
) {
    if (IsZero(x)) {
        // Ideal functionality: uniform output
        return RandomFieldElement();
    }

    ZZ_p s = RandomFieldElement();

    ZZ_p u = RandomFieldElement();

    ZZ_p x_inv = inv(x);
    ZZ_p t = u * x_inv + s;

    // second OLE
    ZZ_p c = (t + a) * x + (b - u);

    return c - s * x;
}


