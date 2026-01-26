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

