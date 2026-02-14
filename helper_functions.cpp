#include "helper_functions.h"

NTL::ZZ messageToZZ(const std::string& m) {
        NTL::ZZ z(0);
        for (unsigned char c : m) {
                z <<= 8;
                z += c;
        }
        return z;
}

std::string ZZToMessage(NTL::ZZ z) {
        std::string m;
        while (z > 0) {
                unsigned char b = NTL::conv<unsigned long>(z & 0xFF);
                m.insert(m.begin(), b);
                z >>= 8;
        }
        return m;
}

size_t FieldByteLength() {
	ZZ p = ZZ_p::modulus();
	// dividing canonical representation, truncates given size_t datatype
	return (NumBits(p) + 7) / 8; 
};

ZZ EncodeZZ_p(const ZZ_p& zz_p) {
	ZZ zz = rep(zz_p);
	ZZ out = ZZ(0);
	size_t bitsNum = NumBits(ZZ_p::modulus());
	size_t bytesNum = (bitsNum + 7) /8;

	for (int i = bytesNum; i >= 0; i--) {
		// extracting i-th byte from the right, similar to above conversions
		ZZ byte = (zz >> (8*i)) & 0xFF; // start from highest i to get rightmost byte
		out += byte; 
	}
	return out;
};

// used mainly in PRF to accept ZZ_p keys (check prf.cpp/h)
SecByteBlock EncodeKeyZZ(const ZZ k) {
	ZZ zz = k; // was previously ZZ_p, simplify later
	size_t bitsNum = NumBits(ZZ_p::modulus());
	size_t bytesNum = (bitsNum + 7) / 8;

	SecByteBlock k_encoded(bytesNum); // secbyteblock, useful buffer, check docs for more
	for (size_t i = 0; i < bytesNum; i++) {
		// start from the last element, as low bits are processed first
		k_encoded[bytesNum - 1 - i] = conv<unsigned long>(zz & 0xFF);
		zz >>= 8;
	}
	return k_encoded;
};

ZZ_p EncodeAsPoly(ZZ x, ZZ m) {
	ZZ_p pi = to_ZZ_p(x + m);
	return pi;
};

ZZ_p EncodeAsPoly(ZZ_p x, ZZ m) {
	ZZ_p pi = x + to_ZZ_p(m);
	return pi;
};

