#include "solve_puzzles.h"

// evalPzl
SolvePuzzle::SolvePuzzle(
		int cmd, 
		Vec<ZZ_p> puzzle_vector, 
		Vec<Vec<ZZ>> PP_eval, 
		Vec<Vec<ZZ>> PP, 
		ZZ p, 
		Vec<ZZ_p> X, 
		int t, 
		std::vector<int> leader_indices) : 
	cmd(std::move(cmd)), 
	puzzle_vector(std::move(puzzle_vector)), 
	PP_eval(std::move(PP_eval)), 
	PP(std::move(PP)),
	p(std::move(p)), 
	X(std::move(X)), 
	t(std::move(t)),
	leader_indices(std::move(leader_indices)) {
		assert(cmd != 0);
	};

// clientPzl
SolvePuzzle::SolvePuzzle(
		int cmd, 
		Vec<ZZ_p> puzzle_vector, 
		int targetClient,
		Vec<Vec<ZZ>> PP, 
		ZZ p, 
		Vec<ZZ_p> X, 
		int t) : 
	cmd(std::move(cmd)), 
	puzzle_vector(std::move(puzzle_vector)), 
	targetClient(std::move(targetClient)),
	PP(std::move(PP)),
	p(std::move(p)), 
	X(std::move(X)), 
	t(std::move(t)) {
		assert(cmd == 0);
	};

// PzlEval

void SolvePuzzle::g_findSecretKeys() {
	std::cout << "[SolvePuzzle] Finding secret keys" << std::endl;
	tK.SetLength(3);
	Vec<ZZ> tmp_tks;
	Vec<ZZ> tmp_k_prime;
	Vec<ZZ> tmp_s_prime;
	for (int i = 0; i < t; i++) {
		int leader = leader_indices[i];
		// extracting puzzle parameters
		ZZ h, tk, Y, N;
		Y = PP_eval[1][i];
		N = PP[3][leader];
		h = PP_eval[0][i]; // initially base h, h^(2^Y) to get tk
		tk = h;

		for (int j = 0; j < Y; j++)
			tk = MulMod(tk, tk, N);

		tmp_tks.append(tk);

		ZZ k_prime_u = PRF_AES(conv<ZZ_p>(1), tk);
		ZZ s_prime_u = PRF_AES(conv<ZZ_p>(2), tk);

		tmp_k_prime.append(k_prime_u);
		tmp_s_prime.append(s_prime_u);
	}
	tK[0] = tmp_tks;
	tK[1] = tmp_k_prime;
	tK[2] = tmp_s_prime;
}

void SolvePuzzle::g_removeBlindFactors() {
	std::cout << "[SolvePuzzle] Removing blind factors" << std::endl;
	std::cout << "[SolvePuzzle] (Sanity check) Prime p when solving: " << p << std::endl; 
	std::cout << "[SolvePuzzle] (Sanity check) Global modulus: " << ZZ_p::modulus() << std::endl;
	Vec<ZZ_p> tmp_theta;
	for (int i = 0; i < puzzle_vector.length(); i++) {
		ZZ_p product = conv<ZZ_p>(1);
		for (int j = 0; j < t; j++) {
			product *= to_ZZ_p(PRF_AES(conv<ZZ_p>(i), tK[2][j])); // w_prime
		}
		product = inv(product);

		ZZ_p sum = conv<ZZ_p>(0);
		for (int j = 0; j < t; j++) {
			sum += to_ZZ_p(PRF_AES(conv<ZZ_p>(i), tK[1][j])); // z-prime
		}
		ZZ_p theta = product * (puzzle_vector[i] - sum);
		tmp_theta.append(theta);
	}
	theta = tmp_theta;
}

void SolvePuzzle::g_extractPolynomial() {
	std::cout << "[SolvePuzzle] Extracting Polynomial" << std::endl;
	
	// check for correctness
	Vec<ZZ_p> extracted_roots = interpolate_roots(X, theta);
	for (int u = 0; u < extracted_roots.length(); u++) {
   		ZZ_p val = evaluate_and_interpolate(X, theta, extracted_roots[u]);
    		std::cout << "[SolvePuzzle] theta(root["<<u<<"]) = " << val << "\n";
	}

	// debug
	//ZZ_pX P = interpolate_polynomial(X, theta);
	//cons = eval(P, ZZ_p(0));
	//Vec<ZZ_p> extracted_roots = interpolate_roots(X, theta);
	//std::cout << "extracted roots: " << extracted_roots << std::endl;
	
	// interpolate and evaluate at point 0, extracting the constant of theta(x) as presented on p.24 (Part 5, step c, detailed construction)
	cons = evaluate_and_interpolate(X, theta);
	std::cout << "[SolvePuzzle] cons: " << cons << std::endl;
};

void SolvePuzzle::g_extractLinearCombination() {
	std::cout << "[SolvePuzzle] Extracting linear combination" << std::endl;
	ZZ_p product = ZZ_p(1);
	for (int i = 0; i < roots.length(); i++) {
		product *= -roots[i];	
	}
	res = cons * inv(product); 
};

void SolvePuzzle::g_extractValidRoots() {
	std::cout << "[SolvePuzzle] Extracting valid roots" << std::endl;
	Vec<ZZ_p> tmp_roots;
	Vec<Vec<ZZ>> tmp_proof;
	tmp_proof.SetLength(2);

	Vec<ZZ_p> extracted_roots = interpolate_roots(X, theta);	
	std::cout << "[SolvePuzzle] extracted roots: " << extracted_roots << std::endl;
	for (int i = 0; i < extracted_roots.length(); i++) {
		for (int j = 0; j < tK[0].length(); j++) {
			ZZ comm = commit(rep(extracted_roots[i]), tK[0][j]);
			if (comm == PP_eval[2][j]) {
				tmp_roots.append(extracted_roots[i]);
				tmp_proof[0].append(rep(extracted_roots[i]));
				tmp_proof[1].append(tK[0][j]);
			}
		}
	}

	roots = tmp_roots;
	g_proof = tmp_proof;
};

void SolvePuzzle::g_publish() {
	std::cout << "[SolvePuzzle] Publishing" << std::endl;
	g_output = res;
}

void SolvePuzzle::g_solve() {
	g_findSecretKeys();
	g_removeBlindFactors();
	g_extractPolynomial();
	g_extractValidRoots();
	g_extractLinearCombination();
	g_publish();

	std::cout << "\n[SolvePuzzle] PzlEval: " << g_output << std::endl;
}

// clientPzl
void SolvePuzzle::o_findSecretKeys() {
	std::cout << "\n[SolvePuzzle] Finding secret keys" << std::endl;
	// grab corrsponding pp_u
	ZZ T = PP[1][targetClient];
	ZZ r = PP[2][targetClient];
	ZZ N = PP[3][targetClient];

	ZZ mk = r;
	// find mk
	for (int i = 0; i < T; i++)
		mk = MulMod(mk, mk, N);
	K.append(mk);

	ZZ k = PRF_AES(ZZ_p(1), mk);
	ZZ s = PRF_AES(ZZ_p(2), mk);
	K.append(k);
	K.append(s);
}

void SolvePuzzle::o_removeBlindFactors() {
	std::cout << "[SolvePuzzle] Removing blinding factors" << std::endl;
	Vec<Vec<ZZ_p>> clientBlindingFactors;
	clientBlindingFactors.SetLength(2);
	for (int i = 0; i < t + 2; i++) {
		ZZ_p z = to_ZZ_p(PRF_AES(conv<ZZ_p>(i), K[1]));
		ZZ_p w = to_ZZ_p(PRF_AES(conv<ZZ_p>(i), K[2]));
		clientBlindingFactors[0].append(z);
		clientBlindingFactors[1].append(w);
	}
	// unblind
	Vec<ZZ_p> tmp_pi;
	for (int i = 0; i < t + 2; i++) {
		ZZ_p pi_i = (inv(clientBlindingFactors[1][i]) * puzzle_vector[i]) - clientBlindingFactors[0][i];
		tmp_pi.append(pi_i);
	}
	pi = tmp_pi;
}

void SolvePuzzle::o_extract_and_publish() {
	std::cout << "[SolvePuzzle] Extracting and publishing" << std::endl;
	ZZ_p m = evaluate_and_interpolate(X, pi); // considers the constant term of pi_u as the plaintext solution
	o_output = m;
	o_proof = K[0];
}

void SolvePuzzle::o_solve() {
	o_findSecretKeys();
	o_removeBlindFactors();
	o_extract_and_publish();
	
	std::cout << "\n[SolvePuzzle] clientPzl: " << o_output << std::endl;
}


