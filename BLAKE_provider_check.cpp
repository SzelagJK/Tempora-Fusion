#include <cryptopp/blake3.h>
#include <iostream>

int main() {
    CryptoPP::BLAKE3 hash;
    std::cout << "BLAKE3 provider: " << hash.AlgorithmProvider() << std::endl;
    // Output: "AVX512", "AVX2", "SSE4.1", "NEON", or "C++"
    return 0;
}

// remember to specify the link with /usr/local/lib/libcryptopp.a (or similar) in case *if* /usr/lib is found first in the linker search
