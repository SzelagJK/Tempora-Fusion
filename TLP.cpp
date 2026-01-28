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

#include "poly_field.h"
#include "OLE.h"
#include "rsa.h"
#include "helper_functions.h"
#include "tlp.h"

using namespace NTL;
using namespace CryptoPP;

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

void testPolynomialGeneration() {
	std::cout << "[TEST] Polynomial Generation\n";
    	ZZ testPrime = GenPrime_ZZ(512);
    	std::cout << "Field prime (PolyGen): " << testPrime << std::endl;
    	Poly_Field PF = Poly_Field(testPrime, 1);
	FirstDegPolynomial P1 = FirstDegPolynomial(PF);
    	std::cout << "Polynomial Degree: " << PF.getDegree() << std::endl;
    	std::cout << "Prime Bits (m): " << PF.getBits() << std::endl;
    	// std::cout << "Base 2 T cardinality: " << PF.getLog2T() << std::endl; // Suspiciously big, check later
	std::cout << "Generated Polynomial: " << P1.P[0] << " " << P1.P[1] << "x" << std::endl; 
	std::cout << "Evaluation at x*: " << evaluate_deg1(P1.P, conv<ZZ_p>(1234)) << std::endl;
	std::cout << "	OK\n";
}

void testOLEInterface() {
	std::cout << "[TEST] OLE Interface\n";
	ZZ testPrime = GenPrime_ZZ(512);
	std::cout << "Field prime (OLE Interface): " << testPrime << std::endl;
	Poly_Field PF = Poly_Field(testPrime, 1);
	FirstDegPolynomial P = FirstDegPolynomial(PF);
	int d = PF.getDegree();
	long bits = PF.getBits();
	ZZ log2T = PF.getLog2T();
	// When implementing unit test, start here
	
	ZZ tau = conv<ZZ>(10);
	OLE_Interface OLE = OLE_Interface(tau, log2T);
	std::cout << "Checkpoint: Interface Created" << std::endl;
	Vec<long> alpha = generateAlpha(OLE);
	std::cout << "Checkpoint: Alpha generated" << std::endl;
	Vec<Vec<ZZ_p>> R = generateR(OLE, PF, alpha);
	std::cout << "Checkpoint: R generated" << std::endl;
	Vec<ZZ_p> h = hRP(OLE, P, R, alpha);

	std::cout << "h: " << h[0] << " " << h[1] << "x" << std::endl;

	Vec<ZZ_p> r = generateVector_r(OLE);
	std::cout << "r check: " << r[1] << std::endl;

	Vec<Vec<ZZ_p>> pairs = PreparePairs(r, R);
	std::cout << "pairs check: " << pairs[1][1];
}

int main() {
    std::cout << "Running TLP tests...\n\n";

    testRSASetup();
    testTrapdoorVsSequential();
    testSequentialTimingStatistical();
    testEndToEndPuzzle();

    testPolynomialGeneration();
    testOLEInterface();

    std::cout << "\nAll tests passed.\n";
    return 0;
}

// NOTES for next time I'll pick this up: separate functions into C headers to use them across different files (RSA key generation, Puzzle generation, TLP functions, helper functions)
// Start putting everything together with respect to the paper, look over OLE and PRF implementations to see if they are correct and can be used.
// Keep in mind that the plaintext message will be first encoded as a polynomial, so we can keep the Message to ZZ and its inverse- but we need to add additional functions that encode ZZ into a polynomial (plus interpolation)
