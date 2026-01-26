#include <NTL/ZZ.h>
//#include <gmpxx.h> // gmpxx not found, fix at a later point  (NTL works)
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>
#include <cryptopp/filters.h>
#include <cassert>
#include <chrono>
#include <vector>
#include <string>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace NTL;
using namespace CryptoPP;

// Helper functions
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

// immutable params
class RSAParams {
	private:
		const ZZ N, phi;
	public:
		RSAParams(ZZ N, ZZ phi) : N(std::move(N)), phi(std::move(phi)) {}
		const ZZ& getN() const {return N;}
		const ZZ& getPhi() const {return phi;}
};

// RSA setup
RSAParams setupRSA(long lambda) {
	ZZ p = GenPrime_ZZ(lambda/2);
	ZZ q = GenPrime_ZZ(lambda/2);
	ZZ N = p*q;
        ZZ phi = (p-1)*(q-1);
	return RSAParams(N, phi);	
}

class TLPPuzzle {
	private:
		ZZ N, r, c;
		long T;
	public:
		TLPPuzzle(ZZ N, ZZ r, long T, ZZ c) : N(std::move(N)), r(std::move(r)), T(std::move(T)), c(std::move(c)) {};
		const ZZ& getR() const {return r;}
		const long& getT() const {return T;}
		const ZZ& getCiphertext() const {return c;}
};

TLPPuzzle generatePuzzle(
    const RSAParams &params,
    const std::string &message,
    long T
) {
    ZZ r = RandomBnd(params.getN());
    ZZ a = PowerMod(ZZ(2), T, params.getPhi());
    ZZ b = PowerMod(r, a, params.getN()); // b = a^2^T 

    ZZ encodedMessage = messageToZZ(message);
    ZZ maskedMessage = encodedMessage + b % params.getN();
    std::cout << "Masked messsage: " << maskedMessage;
    TLPPuzzle o = TLPPuzzle(
		    params.getN(),
		    r,
		    T,
		    maskedMessage
		    );
    std::cout << o.getCiphertext() << std::endl;
    return o;
}

std::string solvePuzzle(const TLPPuzzle &o, RSAParams params) {
    ZZ b = o.getR();
    for (long i = 0; i < o.getT(); i++) { // note to self: sequentially compute the mask of the message to reveal it
        b = PowerMod(b, 2, params.getN());
    }

    ZZ unmaskedPuzzle = o.getCiphertext() - b;

    return ZZToMessage(unmaskedPuzzle);
}

// TESTS

void testRSASetup() {
    std::cout << "[TEST] RSA setup...\n";
    RSAParams params = setupRSA(512);

    assert(params.getN() > 0);
    assert(params.getPhi() > 0);

    std::cout << "N: " << params.getN() << std::endl;
    std::cout << "Phi: " << params.getPhi() << std::endl;

    ZZ test = PowerMod(ZZ(2), params.getPhi(), params.getN());
    assert(test == 1 || GCD(ZZ(2), params.getN()) != 1);

    std::cout << "  OK\n";
}

void testTrapdoorVsSequential() {
    std::cout << "[TEST] Trapdoor vs sequential squaring...\n";
    RSAParams params = setupRSA(512);

    long T = 50;
    ZZ r = RandomBnd(params.getN());

    ZZ a = PowerMod(ZZ(2), T, params.getPhi());
    ZZ fast = PowerMod(r, a, params.getN());

    ZZ slow = r;
    for (long i = 0; i < T; i++)
        slow = PowerMod(slow, 2, params.getN());

    assert(fast == slow);
    std::cout << "  OK\n";
}

void testSequentialTimingStatistical() {
    std::cout << "[TEST] Sequential delay (statistical)...\n";

    RSAParams params = setupRSA(512);
    ZZ r = RandomBnd(params.getN());

    const int trials = 20;
    long T1 = 200;
    long T2 = 400;

    long long sum1 = 0, sum2 = 0;

    for (int i = 0; i < trials; i++) {
        auto s1 = std::chrono::high_resolution_clock::now();
        ZZ x = r;
        for (long j = 0; j < T1; j++)
            x = PowerMod(x, 2, params.getN());
        auto e1 = std::chrono::high_resolution_clock::now();
        sum1 += std::chrono::duration_cast<std::chrono::microseconds>(e1 - s1).count();

        auto s2 = std::chrono::high_resolution_clock::now();
        x = r;
        for (long j = 0; j < T2; j++)
            x = PowerMod(x, 2, params.getN());
        auto e2 = std::chrono::high_resolution_clock::now();
        sum2 += std::chrono::duration_cast<std::chrono::microseconds>(e2 - s2).count();
    }

    assert(sum2 > sum1);
    std::cout << "  OK\n";
}


void testEndToEndPuzzle() {
    std::cout << "[TEST] End-to-end puzzle...\n";

    RSAParams params = setupRSA(512);
    std::string message = "Time-lock puzzle test message";
    long T = 36000;

    TLPPuzzle o = generatePuzzle(params, message, T);
    std::string recovered = solvePuzzle(o, params);

    assert(recovered == message);
    std::cout << "  OK\n";
}

int main() {
    std::cout << "Running TLP tests...\n\n";

    testRSASetup();
    testTrapdoorVsSequential();
    testSequentialTimingStatistical();
    testEndToEndPuzzle();

    std::cout << "\nAll tests passed.\n";
    return 0;
}

// NOTES for next time I'll pick this up: separate functions into C headers to use them across different files (RSA key generation, Puzzle generation, TLP functions, helper functions)
// Start putting everything together with respect to the paper, look over OLE and PRF implementations to see if they are correct and can be used.
// Keep in mind that the plaintext message will be first encoded as a polynomial, so we can keep the Message to ZZ and its inverse- but we need to add additional functions that encode ZZ into a polynomial (plus interpolation)
