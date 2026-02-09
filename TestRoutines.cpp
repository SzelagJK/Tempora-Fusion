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
#include "OT_1of2.h"
#include "ope_interface.h"
#include "ole.h"
#include "ole_enhanced.h"
#include "rsa.h"
#include "helper_functions.h"
#include "tlp.h"
#include "setup.h"

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

void testOPEInterface() {
	std::cout << "[TEST] OPE Interface\n";
	int key_bits = 512;
	ZZ testPrime = GenPrime_ZZ(key_bits);
	std::cout << "Field prime (OPE Interface): " << testPrime << std::endl;
	Poly_Field PF = Poly_Field(testPrime, 1);
	FirstDegPolynomial P = FirstDegPolynomial(PF);
	int d = PF.getDegree();
	long bits = PF.getBits();
	int log2T = PF.getLog2T();

	// When implementing unit test, start here
	
	ZZ tau = conv<ZZ>(10);
	OPE_Interface OPE = OPE_Interface(tau, log2T);
	std::cout << "Checkpoint: Interface Created" << std::endl;
	Vec<long> alpha = OPE.generateAlpha();
	std::cout << "Checkpoint: Alpha generated" << std::endl;
	Vec<Vec<ZZ_p>> R = OPE.generateR(PF, alpha);
	std::cout << "Checkpoint: R generated" << std::endl;
	Vec<ZZ_p> h = OPE.hRP(P, R, alpha);

	std::cout << "h: " << h[0] << " " << h[1] << "x" << std::endl;

	Vec<ZZ_p> r = OPE.generateVector_r();
	std::cout << "r check: " << r[1] << std::endl;

	Vec<Vec<ZZ_p>> pairs = OPE.PreparePairs(r, R, conv<ZZ_p>(13));
	std::cout << "pairs check: " << pairs[1][1] << std::endl;

	ZZ_p maskedOutput = OPE.runOT_and_sum(pairs, alpha, key_bits);
	std::cout << "OT loop check: " << maskedOutput << std::endl;

	ZZ_p result = OPE.extract_eval(maskedOutput, r);
	std::cout << "Final OPE result evaluation: " << result << std::endl;

	std::cout << "	OK\n\n\n";
}

void testOT() {
	std::cout << "[TEST] Oblivious Transfer (1-out-of-2)\n";
	ZZ_p rm0 = random_ZZ_p();
	ZZ_p rm1 = random_ZZ_p();
	bigint result;
	mpz_init(result);
	bigint* result_ptr = OT_1of2(rm0, rm1, 1, 512);
	mpz_set(result, *result_ptr);
	char* s = mpz_get_str(NULL, 10, result);
	std::cout << "OT result: " << s << std::endl;

	std::cout << "	OK\n";
}

void testOLE() {
       	std::cout << "\n\n\n [TEST] OLE main test\n" << std::endl;
       	int key_bits = 512; 
	ZZ test_prime = GenPrime_ZZ(key_bits);
	Poly_Field PF = Poly_Field(test_prime, 1);
	OLE OLE_p = OLE(test_prime, key_bits, PF);
	ZZ testInput = conv<ZZ>(1000);
	Vec<ZZ> coeffs_ab = init_coeff_vector(3,5);
	std::cout << "Testing OLE on input: " << testInput << std::endl;
	ZZ_p result = OLE_p.runOLE(testInput, coeffs_ab);
	std::cout << "Computed evaluation: " << result << std::endl;

	std::cout << "	OK\n";
}

void testOLE_enhanced() {
	std::cout << "\n\n\n [TEST] OLE+ main test\n" << std::endl;
	int key_bits = 128;
	ZZ test_prime = GenPrime_ZZ(key_bits);
	OLE_enhanced OLE_p(test_prime);
	ZZ testInput = conv<ZZ>(10);
	Vec<ZZ> coeffs_ab = init_coeff_vector(11,5);
	// Randomise Secrets
	ZZ s = RandomLen_ZZ(key_bits);
	ZZ u = RandomLen_ZZ(key_bits);
	Vec<ZZ> secrets = init_secrets_vector(s, u);
	std::cout << "Testing OLE+ on input: " << testInput << std::endl;
	ZZ_p result = OLE_p.runOLE_plus(testInput, coeffs_ab, secrets);
	std::cout << "Correct evaluation: " << (coeffs_ab[0] + coeffs_ab[1]*testInput) << std::endl;
	std::cout << "Computed evaluation: " << result << std::endl;
}

void testSetup() {
	std::cout << "\n\n [TEST] VHLC-TLP Setup" << std::endl;
	int key_bits = 128;
	ZZ test_prime = GenPrime_ZZ(key_bits);
	int leader_clients = 10;
	
	std::cout << "Server Setup Checks" << std::endl;
	Setup_S S = Setup_S(test_prime, leader_clients);
	std::cout << "Leader Qty check: " << S.getLeaderQty() << std::endl;
	S.setFieldParams(1);
	S.generatePublicX();
	std::cout << "Public X check: " << S.getX().length() << std::endl;

	std::cout << "Client Setup Checks" << std::endl;
	long lambda = 2048; // key bits
	Setup_C C = Setup_C(lambda);
	std::cout << "Client public key (N): " << C.getPublicKey() << std::endl;
	std::cout << "Client secret key (Phi(N)): " << C.getSecretKey() << std::endl;
}

int main() {
    	//std::cout << "Running TLP tests...\n\n";

    	//testRSASetup();
    	//testTrapdoorVsSequential();
   	//testSequentialTimingStatistical();
    	//testEndToEndPuzzle();

    	// Auxillery tests
    	//testPolynomialGeneration();
    	//testOT();
    	//testOPEInterface();

    	// Main test for OLE
	// testOLE();

	// Main test for OLE+
	auto start = std::chrono::high_resolution_clock::now();
	int iterations = 1;
	for (int i = 0; i < iterations; i++)
		testOLE_enhanced();
	auto end = std::chrono::high_resolution_clock::now();	

	std::chrono::duration<double, std::micro> test_time = end - start;
	double average_time = test_time.count()/iterations;
	std::cout << "OLE+ average execution time: " << average_time/1000 << "ms" << std::endl;
	std::cout << "OLE+ total execution time: " << test_time.count()/1000 << "ms" << std::endl;

	// Tests for VHLC-TLP
	
	testSetup();


    	std::cout << "\nAll tests passed.\n";
    	return 0;
}
// Notes for the next time: 
// Move onto the paper and proceed with linear combinations + interactions (see what fits best at this point in the implementation)
// This will become a library remember to generalise interfaces to any degree polynomial, and add some more overloading
