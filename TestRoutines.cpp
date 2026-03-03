#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/osrng.h>
#include <cryptopp/filters.h>
#include <NTL/BasicThreadPool.h>
#include <cassert>
#include <chrono>
#include <vector>
#include <string>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <set>
#include <cstdlib>

#include "poly_field.h"
#include "OT_1of2.h"
#include "ope_interface.h"
#include "ole.h"
#include "ole_enhanced.h"
#include "rsa.h"
#include "helper_functions.h"
#include "tlp.h"
#include "prf.h"
#include "commitment.h"
#include "setup.h"
#include "gen_puzzle.h"
#include "coin_toss.h"
#include "linear_comb.h"
#include "poly_interpolate.h"
#include "solve_puzzles.h"
#include "verify.h"

using namespace NTL;
using namespace CryptoPP;

/* 
 *	TESTS AND EXAMPLES
 * 	
 * 	Use for both tests and reference for usage, each funciton has been written with intentions of running independently.
 *
 * 	(the main exceptions are any functions that involve ZZ_p, where global modulus needs to be initialised,
 * 	in that case, run testPolynomialGeneration() beforehand which will automatically initialise the global modulus and a
 * 	polynomial field)
 *
 * 	To run the entire script, use:
g++ gen_puzzle.cpp ope_interface.cpp poly_field.cpp rsa.cpp tlp.cpp commitment.cpp ole.cpp OT_1of2.cpp TestRoutines.cpp helper_functions.cpp ole_enhanced.cpp prf.cpp setup.cpp coin_toss.cpp linear_comb.cpp poly_interpolate.cpp solve_puzzles.cpp verify.cpp /usr/local/lib/libcryptopp.a -lntl -lgmp -lcryptopp -o VHLC_TLP.exe
 * 	
 * 	where /usr/local/lib/libcryptopp.a points to an .a file that was built form modern-cryptopp (for BLAKE3)
 */


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
        std::cout << "\n\n[TEST] Oblivious Transfer (1-out-of-2)\n";

        // Ensure zero is initialized (OT helpers)
        mpz_init_set_ui(zero, 0);

        const int iterations = 1000;
        const int key_bits = 128;
        const int choice = 1;

        using clock = std::chrono::high_resolution_clock;

        mpz_t result;
        mpz_init(result);

        bool all_correct = true;

        auto start = clock::now();

        for (int i = 0; i < iterations; i++) {

                ZZ_p rm0 = random_ZZ_p();
                ZZ_p rm1 = random_ZZ_p();

                bigint* result_ptr = OT_1of2(rm0, rm1, choice, key_bits);

                mpz_set(result, *result_ptr);

                // Lift expected value to mpz for comparison
                mpz_t expected;
                mpz_init(expected);
                ZZp_to_mpz(expected, (choice == 0 ? rm0 : rm1));

                if (mpz_cmp(result, expected) != 0) {
                        all_correct = false;
                }

                mpz_clear(expected);

                // Self note:
                // OT_1of2 internally allocates matrices and arrays without providing cleanup
        }

        auto end = clock::now();

        std::chrono::duration<double, std::micro> test_time = end - start;
        double average_time = test_time.count() / iterations;

        std::cout << "OT_1of2 average execution time: \033[1;38;5;208m"
                  << average_time / 1000 << "ms\033[0m" << std::endl;

        std::cout << "OT_1of2 total execution time: \033[1;38;5;208m"
                  << test_time.count() / 1000 << "ms\033[0m" << std::endl;

        if (all_correct)
                std::cout << "        PASS: All 1000 OT executions correct\n";
        else
                std::cout << "        FAIL: At least one OT execution incorrect\n";

        mpz_clear(result);

        std::cout << "        OK\n";
}

void testOLE() {
        using clock = std::chrono::high_resolution_clock;

        std::cout << "\n\n[TEST] OLE main test\n" << std::endl;
        int key_bits = 128; 
        ZZ test_prime = GenPrime_ZZ(key_bits);
        Poly_Field PF = Poly_Field(test_prime, 1);
        OLE OLE_p = OLE(test_prime, key_bits, PF);
        ZZ testInput = conv<ZZ>(1000);
        Vec<ZZ> coeffs_ab = init_coeff_vector(3,5);
        std::cout << "Testing OLE on input: " << testInput << std::endl;

        ZZ_p result;

        auto total_start = clock::now();

	int iterations = 1000;
        for (int i = 0; i < iterations; i++) {
                result = OLE_p.runOLE(testInput, coeffs_ab);
        }

        auto total_end = clock::now();

        std::chrono::duration<double, std::micro> total_time_us = total_end - total_start;
        std::chrono::duration<double, std::micro> avg_time_us = total_time_us / iterations;

        std::cout << "Computed evaluation: " << result << std::endl;

        std::cout << "OLE_p.runOLE avg. execution time: \033[1;38;5;208m"
                  << avg_time_us.count() / 1000 << "ms\033[0m" << std::endl;

        std::cout << "  OK\n";
}

void testOLE_enhanced() {
	std::cout << "\n\n[TEST] OLE+ main test\n" << std::endl;
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
	// unit testing
	using clock = std::chrono::high_resolution_clock;
        ZZ_p result;
        int iterations = 1000;  
        auto start = clock::now();
        for (int i = 0; i < iterations; i++) {
                OLE_p.runOLE_plus(testInput, coeffs_ab, secrets);
        }
        auto end = clock::now();
        std::chrono::duration<double, std::micro> test_time = end - start;
        double average_time = test_time.count() / iterations;

        std::cout << "OLE+ average execution time: \033[1;38;5;208m"
                  << average_time / 1000 << "ms\033[0m" << std::endl;
        std::cout << "OLE+ total execution time: \033[1;38;5;208m"
                  << test_time.count() / 1000 << "ms\033[0m" << std::endl;	

	std::cout << "Correct evaluation: " << (coeffs_ab[0] + coeffs_ab[1]*testInput) << std::endl;
	std::cout << "Computed evaluation: " << result << std::endl;
}

void testPRF() {
	std::cout << "\n\n[TEST] testing PRF" << std::endl;
	int key_bits = 128;

	int testIterations = 1000;
	int pass = 0;
	int fail = 0;
	ZZ test_prime = GenPrime_ZZ(key_bits);
	ZZ key = test_prime;
	auto start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < testIterations; i++) {
		ZZ_p input1 = to_ZZ_p(conv<ZZ>(1));
		ZZ_p input2 = to_ZZ_p(conv<ZZ>(2));
		ZZ r1 = PRF_AES(input1, key);
		//std::cout << "R1: " << r1 << std::endl;
		//std::cout << "R1 bits: " << NumBits(r1) << std::endl;
		
		ZZ r1_check = PRF_AES(input1, key);
		//std::cout << "R1 check: " << r1_check << std::endl;
		
		ZZ r2 = PRF_AES(input2, key);
		//std::cout << "R2: " << r2 << std::endl;
		//std::cout << "R2 bits: " << NumBits(r2) << std::endl;

		ZZ r2_check = PRF_AES(input2, key);
		//std::cout << "R2 check: " << r2_check << std::endl;
		if (r1_check == r1 && r2_check == r2) {
			//std::cout << "PASS" << std::endl;
			pass += 1;
		} else {
			//std::cout << "FAIL" << std::endl;
			fail += 1;
		}
	}
	auto end = std::chrono::high_resolution_clock::now();	

	std::chrono::duration<double, std::micro> test_time = end - start;
	double average_time = test_time.count()/testIterations;
	std::cout << "[PRF] average execution time: \033[1;38;5;208m" << (average_time/1000)/4 << "ms\033[0m" << std::endl;
	std::cout << "[PRF] total execution time: \033[1;38;5;208m" << test_time.count()/1000 << "ms\033[0m" << std::endl;

	std::cout << "[PRF] PASS Count: " << pass << std::endl;
	std::cout << "[PRF] FAIL Count: " << fail << std::endl;
	std::cout << "[PRF] Pass rate (%): " << 100*((double)pass/(pass+fail)) << std::endl;

	// Unit test for a single prf invocation
	auto start2 = std::chrono::high_resolution_clock::now();
	ZZ_p input3 = to_ZZ_p(3);
	int single_prf_iter = 100000;
	for (int i = 0; i < single_prf_iter; i++) {
		ZZ r3 = PRF_AES(input3, key);
	}
	auto end2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::micro> prf_time = end2 - start2;
	double prf_avg = prf_time.count()/single_prf_iter;
	std::cout << "\n[PRF] Avg. single PRF execution time: \033[1;38;5;208m" << prf_avg/1000 << "ms\033[0m" << std::endl; 
	std::cout << "[PRF] Total execution time: \033[1;38;5;208m" << prf_time.count()/1000 << "ms\033[0m" << std::endl;
}

void testHash() {
	std::cout << "\n[TEST] Commitment test" << std::endl;
	ZZ_p x = to_ZZ_p(conv<ZZ>(2234552342));
	ZZ_p r = to_ZZ_p(conv<ZZ>(3001212113));
	int iterations = 10000;
	auto start = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < iterations; i++)
		ZZ h = commit(x, r);
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::micro> time = end - start;
	double avg_time = time.count()/iterations;
	std::cout << "\n[COMM] Avg. signle commitment execution time: \033[1;38;5;208m" << avg_time/1000 << "ms\033[0m" << std::endl;
	std::cout << "[COMM] Total execution time: \033[1;38;5;208m" << time.count()/1000 << "ms\033[0m" << std::endl;
}

void testCoinToss() {
        using clock = std::chrono::high_resolution_clock;

        std::cout << "\n[TEST] Coin toss protocoli\n";

        ZZ r_hat;

        auto total_start = clock::now();

        for (int i = 0; i < 1000; i++) {
                r_hat = coinToss(20, 128);
        }

        auto total_end = clock::now();

        std::chrono::duration<double, std::micro> total_time_us = total_end - total_start;
        std::chrono::duration<double, std::micro> avg_time_us = total_time_us / 1000.0;

        std::vector<int> indices = determineLeaderIndices(10, 20, r_hat);
        std::cout << "[Coin Toss] r_hat: " << r_hat << std::endl;
        std::cout << "[Coin Toss] Leader Clients (indices): ";
        for (int i = 0; i < indices.size(); i++) {
                std::cout << indices[i] << " ";
        }
        std::cout << "          OK\n";

        std::cout << "coinToss avg. execution time: \033[1;38;5;208m"
                  << avg_time_us.count() / 1000 << "ms\033[0m" << std::endl;
}

void testVHLCTLP(int client_count, int leader_count) {
//	assert(client_count > leader_count + 1);

	std::cout << "\n\n[TEST] VHLC-TLP main test" << std::endl;
        int key_bits = 128;
	
        using clock = std::chrono::high_resolution_clock;

        auto total_start = clock::now();

        ZZ test_prime = GenPrime_ZZ(key_bits);
        int leader_clients = leader_count;
        int total_clients = client_count;

        std::cout << "\n[VHLCTLP] Server setup" << std::endl;
        Setup_S S = Setup_S(test_prime, leader_clients);
        std::cout << "[VHLCTLP] Leader quantity check: " << S.getLeaderQty() << std::endl;
        S.setFieldParams(1);
        S.generatePublicX();

        auto total_end = clock::now();

        std::chrono::duration<double, std::micro> total_time_us = total_end - total_start;

        std::cout << "Server setup (total) total execution time: \033[1;38;5;208m"
                  << total_time_us.count() / 1000 << "ms\033[0m" << std::endl;
	
        std::cout << "\n[VHLCTLP] Client setup checks" << std::endl;
        long lambda = 2048; // key bits for the second primitive
        std::vector<Setup_C> clients = setupMultipleClients(lambda, total_clients); // min treshold: t+2

	// Generate Puzzles
	std::cout << "\n[VHLCTLP] Generating puzzles" << std::endl;
	Vec<ZZ> M;
	ZZ_p sum = ZZ_p(0);
	// random m and delta
        for (int i = 0; i < total_clients; i++) {
                ZZ m = RandomBits_ZZ(32);
                M.append(m);
		//std::cout << "m: " << m << std::endl;
		sum += to_ZZ_p(m);
        }
	std::cout << "\033[33m[DEBUG]\033[0m True sum: " << sum << std::endl;
        std::vector<int> delta;
        for (int i = 0; i < total_clients; i++) {
                int d = static_cast<int>(RandomBnd(9) + 1);
                delta.push_back(d);
        }
        int max_ss = 1;
        VHLCTLP_GenPuzzles PuzzleGenerator(M, clients, test_prime, S.getX(), leader_clients, total_clients, delta, max_ss);
        GenPuzzlesOutput output = PuzzleGenerator.generate_and_publish();

	std::cout << "\n[VHLCTLP] Adjusting linear comb inputs" << std::endl;
	// Adjust input for linear combinations
	S_LinearCombInput S_input;
	S_input.o_vectors = output.o_vectors;
	S_input.delta_combination = 1;
	S_input.max_ss = max_ss;
	S_input.PP = PuzzleGenerator.getPRMs().PP;
	S_input.p = test_prime;
	S_input.X = S.getX();
       	S_input.t = leader_clients;
	// do the same for all clients
	std::vector<C_LinearCombInput> C_vector;
	for (int i = 0; i < clients.size(); i++) {
		C_LinearCombInput C_input;
		C_input.delta_puzzle = delta[i];
		C_input.max_ss = max_ss;
		std::vector<Setup_C> K;
		K.push_back(clients[i]);
		C_input.K = K;
		C_input.q = 1; // experimeint purposes
		C_vector.push_back(C_input);
	}
	std::cout << "[VHLCTLP] Setting up OLE+" << std::endl;
	OLE_enhanced OLE_p = OLE_enhanced(test_prime);
	Poly_Field PF = OLE_p.getPF();
	PRMContainer PRMs_input = PuzzleGenerator.getPRMs();
	std::cout << "[VHLCTLP] Generating combination object" << std::endl;
	LinearCombinations LinCombGenerator = LinearCombinations(S_input, C_vector, PRMs_input, leader_clients, total_clients, OLE_p, PF);
	std::cout << "\n[VHLCTLP] Computing combination" << std::endl;
	Vec<ZZ_p> combinedPuzzle = LinCombGenerator.compute_and_publish();
	std::cout << "[VHLCTLP] Puzzle g: " << combinedPuzzle << std::endl;


	Vec<ZZ_p> X_test;
	Vec<ZZ_p> Y_test;
	for (int i = 0; i < 5; i++) {
		X_test.append(ZZ_p(i+1));
		Y_test.append(ZZ_p(2*i));
	}
	ZZ_p eval_inter = evaluate_and_interpolate(X_test, Y_test);

	SolvePuzzle gSolver = SolvePuzzle(1, combinedPuzzle, LinCombGenerator.getPP_eval(), PRMs_input.PP, test_prime, S.getX(), leader_clients, LinCombGenerator.get_leaderIndices());
	gSolver.g_solve();

	int targetClient = 0;
	SolvePuzzle oSolver = SolvePuzzle(0, output.o_vectors[targetClient], targetClient, PRMs_input.PP, test_prime, S.getX(), leader_clients);
	oSolver.o_solve();


	Verify gVerifier = Verify(1, gSolver.g_output, gSolver.g_proof, combinedPuzzle, LinCombGenerator.getPP_eval(), test_prime, S.getX(), leader_clients);
	int g_outcome = gVerifier.g_verify();

	Verify oVerifier = Verify(0, oSolver.o_output, oSolver.o_proof, output.o_vectors[targetClient], PRMs_input.PP[targetClient]);
	int o_outcome = oVerifier.o_verify();
}

int main() {
    	std::cout << "Running Tests\n\n";

    	// Auxillery tests
    	testPolynomialGeneration(); // for ZZ_p used in OT	
    	testOT();

    	// Main test for OLE
	testOLE();

	// Main test for OLE+
	testOLE_enhanced();

	// More auxilelry (requires p field)
	testPRF();

	testHash();

	// Tests for VHLC-TLP

	testCoinToss(); // negligable runtime cost most likley, dont bother with measuring it now 
	
	using clock = std::chrono::high_resolution_clock;
	auto start = clock::now();
	
	testVHLCTLP(10, 3); // Client count, Leader count	

	auto end = clock::now();

	auto protocolTime = std::chrono::duration_cast<std::chrono::microseconds>(end-start);

	std::cout << "Total Tempora-Fusion execution time: \033[1;93m" << protocolTime.count()/1000 << "ms (" << protocolTime.count()/(double)1000000 << "s)\033[0m" << std::endl;

    	std::cout << "\nAll tests passed.\n";
    	return 0;
}
