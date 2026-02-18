#include "linear_comb.h"

LinearCombinations::LinearCombinations(
		S_LinearCombInput S, 
		std::vector<C_LinearCombInput> C_vector, 
		PRMContainer PRMs,
		int t,
		int clientsCount,
		OLE_enhanced OLE_p,
		Poly_Field PF) : 
	S(std::move(S)), 
	C_vector(std::move(C_vector)), 
	PRMs(std::move(PRMs)),
	t(std::move(t)),
	clientsCount(std::move(clientsCount)),
	OLE_p(std::move(OLE_p)),
	PF(std::move(PF)) {};

void LinearCombinations::selectLeaders() {
	std::cout << "[LinearComb] Selecting leaders" << std::endl;
	ZZ r_hat = coinToss(clientsCount);
	std::cout << "r_hat: " << r_hat << std::endl;
	std::cout << "t: " << t << ", clients count: " << clientsCount << std::endl;
	std::vector<int> tmp_leaders = determineLeaderIndices(t, clientsCount, r_hat);	
	selected_leaders = tmp_leaders;
}

void LinearCombinations::grantComputations() {
	std::cout << "[LinearComb] Granting computations (leaders)" << std::endl;
	// generates temporary secret keys
	Vec<Vec<ZZ>> tmp_tK;
	Vec<Vec<ZZ>> tmp_tBlindingFactors;
	Vec<Vec<ZZ_p>> tmp_encryptedRandomRoots;
	tmp_encryptedRandomRoots.SetLength(t);
	Vec<Vec<ZZ>> tmp_BlindingFactors;
	// t+2 of every v and every y for every client
	Vec<Vec<Vec<ZZ_p>>> VY;
	// f_l keys meant for other leaders
	Vec<Vec<ZZ>> tmp_F;
	tmp_F.SetLength(clientsCount);
	for (int i = 0; i < clientsCount; i++) {
		Vec<ZZ> F_u;
		F_u.SetLength(t-1);
		tmp_F[i] = F_u;
	}
	Vec<ZZ> pp_eval_u;

	for (int i = 0; i < t; i++) {
		C_LinearCombInput leader_client = C_vector[selected_leaders[i]];

		Vec<ZZ> tK_u;
		ZZ b;
		ZZ Y = conv<ZZ>(leader_client.delta_puzzle * leader_client.max_ss);
		PowerMod(b, conv<ZZ>(2), Y, rep(leader_client.K[0].getSecretKey()));
		
		ZZ n = rep(leader_client.K[0].getPublicKey());
		ZZ h;
		do {
			h = RandomBits_ZZ(NumBits(n));
			h %= n;
		} while (GCD(h, n) != 1);
                pp_eval_u.append(h);
                pp_eval_u.append(Y);


		ZZ tk;
		PowerMod(tk, h, b, conv<ZZ>(n));

		ZZ k_prime = PRF_AES(conv<ZZ_p>(1), tk);
		ZZ s_prime = PRF_AES(conv<ZZ_p>(2), tk);
		tK_u.append(tk);
		tK_u.append(k_prime);
		tK_u.append(s_prime);

		// Each client generates a key f_l for every other client (outputting n-1 keys)
		// append 1 key to every vector of F to simulate broadcast of every key to each client
		long lambda = NumBits(S.p);
		long poly_lambda = lambda * lambda; // hard-coded adversarial computational power 
		for (int j = 0; j < clientsCount; j++) {
			if (j == clientsCount)
				continue; // skip its own index
			ZZ f_l;
			RandomBits(f_l, poly_lambda);
			tmp_F[j][i] = f_l;
		}

		tmp_tK.append(tK_u);

		// Generate temporary blinding factors
		Vec<ZZ> tfactors;
		for (int j = 1; j <= t+2; j++) {
			ZZ w_prime = PRF_AES(conv<ZZ_p>(j), s_prime);
			tfactors.append(w_prime);
		}
		tmp_tBlindingFactors.append(tfactors);

		// Encrypt random root
		ZZ_p root = random_ZZ_p();
		roots.append(root);
		Vec<ZZ_p> gamma_prime_vector;
		for (int j = 0; j < t+2; j++) {
			ZZ_p gamma = S.X[j] - root; // encodes
			ZZ_p gamma_prime = gamma * to_ZZ_p(tfactors[j]); // encrypts
			gamma_prime_vector.append(gamma_prime);
		}

		tmp_encryptedRandomRoots[i] = gamma_prime_vector;

		// Generate blinding factors
		// regenerate previous factors
		for (int j = 0; j < t+2; j++) {
			Vec<ZZ> zwz;
			ZZ z = PRF_AES(conv<ZZ_p>(j), PRMs.SP[selected_leaders[i]][0]);
			ZZ w = PRF_AES(conv<ZZ_p>(j), PRMs.SP[selected_leaders[i]][1]);
			zwz.append(z);
			zwz.append(w);
			ZZ z_prime = PRF_AES(conv<ZZ_p>(j), k_prime);
			zwz.append(z_prime); // additional factor
			rFactors.append(zwz);
		}
	}


	for (int i = 0; i < t; i++) {
		C_LinearCombInput leader_client = C_vector[selected_leaders[i]];

		// set values v and y
		Vec<Vec<ZZ_p>> vy_factors;
		vy_factors.SetLength(2);
		for (int j = 0; j < t+2; j++) {
			// work out v_i_u
			ZZ_p product_gamma_prime = conv<ZZ_p>(1);
			for (int l = 0; l < t; l++) {
				if (l == j)
					continue;
				product_gamma_prime *= tmp_encryptedRandomRoots[l][j]; // go around all other leaders encrypted roots, grab i-th (or j-th) element
			}
			ZZ_p v = tmp_encryptedRandomRoots[i][j] * product_gamma_prime;
			vy_factors[0].append(v);
			// mod p comes after, if wrong check again
			// first sum, goes over every fresh key generated for every client besides themselves, and for each f_l generates i prf outputs and sums them together
			ZZ_p sum_f = conv<ZZ_p>(0);
			for (int l = 0; l < tmp_F.length(); l++) {
				if (l == i)
					continue; // skip c_u (itself)

				sum_f += to_ZZ_p(PRF_AES(conv<ZZ_p>(j), tmp_F[l][j]));
			}
			// second sum, goes over every f_l recieved by other leader clients, and iterates over them t+2 times generating a PRF value and summing it
			ZZ_p sum_f_dash = conv<ZZ_p>(0);
			for (int l = 0; l < t-1; l++) {
				sum_f_dash += to_ZZ_p(PRF_AES(conv<ZZ_p>(j), tmp_F[selected_leaders[i]][l]));
			}
			ZZ_p y = to_ZZ_p(conv<ZZ>(-1) * rep(sum_f)) + sum_f_dash;

			vy_factors[1].append(y);
		}
		VY.append(vy_factors);


		// OLE+ re-encodings
		Vec<ZZ_p> d_vector;
		for (int j = 0; j < t+2; j++) {
			ZZ_p e = leader_client.q * vy_factors[0][j] * inv(to_ZZ_p(rFactors[j][1]));
			ZZ_p e_prime = to_ZZ_p(conv<ZZ>(-1) * leader_client.q * rep(vy_factors[0][j]) * rFactors[j][0]) + to_ZZ_p(rFactors[j][2]) + vy_factors[1][j];
			Vec<ZZ> ab;
			// inverse order for correct polynomial representation
			ab.append(rep(e_prime));
			ab.append(rep(e));
			Vec<ZZ> su;
			su.append(rep(random_ZZ_p()));
			su.append(rep(random_ZZ_p()));
			ZZ_p d = OLE_p.runOLE_plus(rep(S.o_vectors[selected_leaders[i]][j]), ab, su, PF);
			d_vector.append(d);
		}
		d_vector_leaders.append(d_vector);

		// commit to the root
		ZZ comm_prime = commit(rep(roots[i]), tmp_tK[i][0]);

		pp_eval_u.append(comm_prime);
		PP_Eval.append(pp_eval_u);	
	}

	tK = tmp_tK;
	F = tmp_F;
	encryptedRandomRoots = tmp_encryptedRandomRoots;
}

void LinearCombinations::grantComputations_nonLeader() {
	std::cout << "[LinearComb] Granting computations (non-leaders)" << std::endl;
	for (int i = 0; i < clientsCount; i++) {
		if (std::binary_search(selected_leaders.begin(), selected_leaders.end(), i))
				continue;

		C_LinearCombInput selected_client = C_vector[i];
		// generate blinding factors
		Vec<Vec<ZZ>> ZW;
		ZW.SetLength(2);
		for (int j = 0; j < t+2; j++) {
			ZZ z = PRF_AES(conv<ZZ_p>(j), PRMs.SP[i][0]);
			ZW[0].append(z);
			ZZ w = PRF_AES(conv<ZZ_p>(j), PRMs.SP[i][1]);
			ZW[1].append(w);
		}

		Vec<Vec<ZZ_p>> VY;
		VY.SetLength(2);
		for (int j = 0; j < t+2; j++) {
			ZZ_p v = conv<ZZ_p>(1);
			for (int l = 0; l < encryptedRandomRoots.length(); l++) {
				v *= encryptedRandomRoots[l][j]; 
			}
			VY[0].append(v);

			ZZ_p y = conv<ZZ_p>(0);
			for (int l = 0; l < F[i].length(); l++)
				y += to_ZZ_p(PRF_AES(conv<ZZ_p>(j), F[i][l]));
			VY[1].append(y);
		}

		// re-encode outsourced puzzle
		Vec<ZZ_p> d_vector;
		for (int j = 0; j < t+2; j++) {
			ZZ_p e = selected_client.q * VY[0][j] * inv(to_ZZ_p(ZW[1][j]));
			ZZ_p e_prime = to_ZZ_p(conv<ZZ>(-1) * selected_client.q * rep(VY[0][j]) * ZW[0][j]) + VY[1][j];
			
			Vec<ZZ> ab;
			ab.append(rep(e_prime));
			ab.append(rep(e));
			Vec<ZZ> su;
			su.append(rep(random_ZZ_p()));
			su.append(rep(random_ZZ_p()));
			ZZ_p d = OLE_p.runOLE_plus(rep(S.o_vectors[i][j]), ab, su, PF);
			d_vector.append(d);
		}
		d_vector_nonLeaders.append(d_vector);
	}

};

void LinearCombinations::computeCombination() {
	std::cout << "[LinearComb] Combining puzzles" << std::endl;
	Vec<ZZ_p> tmp_g_vector;
	for (int i = 0; i < t+2; i++) {
		ZZ_p g = conv<ZZ_p>(0);
		for (int j = 0; j < d_vector_leaders.length(); j++) {
			g += d_vector_leaders[j][i];
		}
		for (int j = 0; j < d_vector_nonLeaders.length(); j++) {
			g += d_vector_nonLeaders[j][i];
		}
		tmp_g_vector.append(g);
	}
	g_vector = tmp_g_vector;
};

const Vec<ZZ_p> LinearCombinations::getG_vector() const {return g_vector;};

const Vec<ZZ_p> LinearCombinations::compute_and_publish() {
	selectLeaders();
	grantComputations();
	grantComputations_nonLeader();
	computeCombination();
	return getG_vector();
};


