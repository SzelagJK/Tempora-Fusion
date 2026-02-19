#include "solve_puzzles.h"

SolvePuzzle::SolvePuzzle(int cmd, Vec<ZZ_p> puzzle_vector, Vec<Vec<ZZ>> PP_eval, Vec<Vec<ZZ>> PP, ZZ p, Vec<ZZ_p> X, int t, std::vector<int> leader_indeces) : 
	cmd(std::move(cmd)), 
	puzzle_vector(std::move(puzzle_vector)), 
	PP_eval(std::move(PP_eval)), 
	p(std::move(p)), 
	X(std::move(X)), 
	t(std::move(t)) {
		assert(cmd != 0);
	};

SolvePuzzle::SolvePuzzle(int cmd, Vec<ZZ_p> puzzle_vector, Vec<Vec<ZZ>> PP, ZZ p, Vec<ZZ_p> X, int t, std::vector<int> leader_indices) : 
	cmd(std::move(cmd)), 
	puzzle_vector(std::move(puzzle_vector)), 
	PP_eval(std::move(PP_eval)), 
	p(std::move(p)), 
	X(std::move(X)), 
	t(std::move(t)) {
		assert(cmd == 0);
	};

void SolvePuzzle::g_findSecretKeys() {
	tK.SetLength(3);
	Vec<ZZ> tmp_tks;
	Vec<ZZ> tmp_k_prime;
	Vec<ZZ> tmp_s_prime;
	tmp_tk.SetLength(t);
	for (int i = 0; i < t; i++) {
		int leader = leader_indices[i];
		// extracting puzzle parameters
		ZZ Y, N;
		tk = PP_eval[i][0]; // initially base h, h^(2^Y) to get tk
		Y = PP_eval[i][1];
		N = PP[3][leader];
		
		for (int j = 0; j < Y, j++) {
			tk = PowerMod(tk, 2, N);
		}
		tmp_tks[i] = tk;

		ZZ k_prime_u = PRF_AES(conv<ZZ_p>(1), tk);
		ZZ s_prime_u = PRF_AES(conv<ZZ_p>(2), tk);

		tmp_k_prime[i] = k_prime_u;
		tmp_s_prime[i] = s_prime_u;
	}
	tK[0] = tmp_tks;
	tK[1] = tmp_k_prime;
	tK[2] = tmp_s_prime;
}

void SolvePuzzle::g_removeBlindFactors() {
	Vec<ZZ_p> tmp_theta;
	for (int i = 0; i < puzzle_vector.length(); i++) {
		ZZ_p product = conv<ZZ_p>(1);
		for (int j = 0; j < t; j++) {
			product *= to_ZZ_p(PRF_AES(conv<ZZ_p>(i), tK[2][j])); // w_prime
		}
		product = inv(product);

		ZZ_p sum = conv<ZZ_p>(0);
		for (int j = 0; j < t; j++) {
			sum += to_ZZ_p(PRF_AES(conv<ZZ_p>(i), tK[1][j]));
		}
		ZZ_p theta = product * (puzzle_vector[i] - sum);
		tmp_theta.append(theta);
	}
	theta = tmp_theta;
}

// void SolvePuzzle::g_extractPolynomial() {};

// void SolvePuzzle::extractLinearCombination() {};

// void SolvePuzzle::extractValidRoots() {};


