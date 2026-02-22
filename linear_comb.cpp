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
	std::sort(tmp_leaders.begin(), tmp_leaders.end()); 
	selected_leaders = tmp_leaders;
}

void LinearCombinations::grantComputations() {
	std::cout << "\n[LinearComb] Granting computations (leaders)" << std::endl;

	// Temporary containers for some class variables, exception safety (commit-or-rollback)  
	Vec<Vec<ZZ>> tmp_tK;
	Vec<Vec<ZZ>> tmp_tBlindingFactors;
	Vec<Vec<ZZ_p>> tmp_encryptedRandomRoots;
	tmp_encryptedRandomRoots.SetLength(t);
	Vec<Vec<ZZ>> tmp_BlindingFactors;
	// t+2 of every v and every y for every client
	Vec<Vec<Vec<ZZ_p>>> VY;
	// f_l keys meant for other client (broadcasted strictly by leaders)
	Vec<Vec<ZZ>> tmp_F;
	tmp_F.SetLength(clientsCount);
	for (int i = 0; i < clientsCount; i++) {
		Vec<ZZ> F_u;
		F_u.SetLength(t);
		tmp_F[i] = F_u;
	}
	PP_Eval.SetLength(3);

	// debug
	Y_all.SetLength(t+2);

	// local holders
	Vec<Vec<Vec<ZZ>>> regeneratedFactors; 
	Vec<Vec<ZZ>> fl_holder;
	// unit test these loops
	for (int i = 0; i < t; i++) {
		const C_LinearCombInput& leader_client = C_vector[selected_leaders[i]];

		Vec<ZZ> tK_u;
		ZZ b;
		ZZ Y = conv<ZZ>(S.delta_combination) * conv<ZZ>(S.max_ss); // self note: keep 2^Y < 2^bits(phi(N))
		assert(Y < leader_client.K[0].getSecretKey()); 
		PowerMod(b, conv<ZZ>(2), Y, leader_client.K[0].getSecretKey());
		
		ZZ n = leader_client.K[0].getPublicKey();
		ZZ h;
		do {
			h = RandomBits_ZZ(NumBits(n));
			h %= n;
		} while (GCD(h, n) != 1);
                PP_Eval[0].append(h);
                PP_Eval[1].append(Y);

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
		Vec<ZZ> fl_client_keys; // keys produced by *this* client i
		for (int j = 0; j < clientsCount; j++) {
			if (j == selected_leaders[i])
				continue; // skip its own index
			ZZ f_l;
			RandomBits(f_l, poly_lambda);
			tmp_F[j][i] = f_l;
			fl_client_keys.append(f_l);
		}

		fl_holder.append(fl_client_keys);
		tmp_tK.append(tK_u);

		// Generate temporary blinding factors
		Vec<ZZ> tfactors;
		for (int j = 0; j < t+2; j++) {
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
		Vec<Vec<ZZ>> zwz_u; // vector of all blinding factors that belong to client i
		for (int j = 0; j < t+2; j++) {
			Vec<ZZ> zwz;
			ZZ z = PRF_AES(conv<ZZ_p>(j), PRMs.SP[selected_leaders[i]][0]);
			ZZ w = PRF_AES(conv<ZZ_p>(j), PRMs.SP[selected_leaders[i]][1]);
			zwz.append(z);
			zwz.append(w);
			ZZ z_prime = PRF_AES(conv<ZZ_p>(j), k_prime);
			zwz.append(z_prime); // additional factor, detailed construction, step 4 - b - iv.
			zwz_u.append(zwz);
		}
		regeneratedFactors.append(zwz_u);

		std::cout << "[LinearComb] Fresh keys and factors generated (" << i+1 << "/" << t << ")" << std::endl;
		
	}


	for (int i = 0; i < t; i++) {
		const C_LinearCombInput& leader_client = C_vector[selected_leaders[i]];

		// set values v and y
		Vec<Vec<ZZ_p>> vy_factors;
		vy_factors.SetLength(2);
		for (int j = 0; j < t+2; j++) {
			// work out v_i_u
			ZZ_p product_gamma_prime = conv<ZZ_p>(1);
			for (int l = 0; l < t; l++) {
				if (l == i) 
					continue;
				product_gamma_prime *= tmp_encryptedRandomRoots[l][j]; // go around all other leaders encrypted roots, grab i-th (or j-th) element
			}
			ZZ_p v = tmp_encryptedRandomRoots[i][j] * product_gamma_prime;
			vy_factors[0].append(v);
			ZZ_p v_check = conv<ZZ_p>(1);
			for (int l = 0; l < t; l++) v_check *= tmp_encryptedRandomRoots[l][j];
			if (v_check != vy_factors[0][j]) {
    				std::cout << "v mismatch at leaderPos = " << i << " j = " << j << std::endl;
			}

			// mod p comes after, if wrong check again
			// first sum, goes over every fresh key generated for every client besides themselves, and for each f_l generates i prf outputs and sums them together (alternatively add all of the keys together themselves and then run them through a PRF)
			ZZ_p sum_f = conv<ZZ_p>(0);
			for (int k = 0; k < fl_holder[i].length(); k++)
				sum_f += to_ZZ_p(PRF_AES(conv<ZZ_p>(j), fl_holder[i][k]));
			// second sum, goes over every f_l recieved by other leader clients, and iterates over them t+2 times generating a PRF value and summing it
			ZZ_p sum_f_dash = conv<ZZ_p>(0);
			for (int l = 0; l < t; l++) {
				if (l == i)
					continue;
				sum_f_dash += to_ZZ_p(PRF_AES(conv<ZZ_p>(j), tmp_F[selected_leaders[i]][l]));
			}
			ZZ_p y = (-sum_f) + sum_f_dash;

			vy_factors[1].append(y);
			if (j==0) // debugging
				Y_all[j].append(y);

		}
		

		VY.append(vy_factors);


		// OLE+ re-encodings
		Vec<ZZ_p> d_vector;
		for (int j = 0; j < t+2; j++) {
			ZZ_p e = leader_client.q * vy_factors[0][j] * inv(to_ZZ_p(regeneratedFactors[i][j][1]));
			ZZ_p e_prime = -(leader_client.q * vy_factors[0][j] * to_ZZ_p(regeneratedFactors[i][j][0])) + to_ZZ_p(regeneratedFactors[i][j][2]) + vy_factors[1][j];
			Vec<ZZ> ab = init_coeff_vector(e_prime, e);
			// inverse order for correct polynomial representation
			Vec<ZZ> su;
			su.append(rep(random_ZZ_p()));
			su.append(rep(random_ZZ_p()));
			ZZ_p d = OLE_p.runOLE_plus(rep(S.o_vectors[selected_leaders[i]][j]), ab, su, PF);

			ZZ_p d_expected = e * to_ZZ_p(rep(S.o_vectors[selected_leaders[i]][j])) + e_prime;
			if (d != d_expected) std::cout << "OLE mismatch at (i=" << i << ", j=" << j << ")" << std::endl;

			d_vector.append(d);
		}
		d_vector_leaders.append(d_vector);

		// commit to the root
		ZZ comm_prime = commit(rep(roots[i]), tmp_tK[i][0]);

		PP_Eval[2].append(comm_prime);

		std::cout << "[LinearComb] Computations (leader) granted (" << i+1 << "/" << t << ")" << std::endl;
	}

	tK = tmp_tK;
	F = tmp_F;
	encryptedRandomRoots = tmp_encryptedRandomRoots;
}

void LinearCombinations::grantComputations_nonLeader() {
	std::cout << "\n[LinearComb] Granting computations (non-leaders)" << std::endl;
	int c = 1; // for terminal output only
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

		Vec<Vec<ZZ_p>> VY_cli;
		VY_cli.SetLength(2);
		for (int j = 0; j < t+2; j++) {
			ZZ_p v = conv<ZZ_p>(1);
			for (int l = 0; l < encryptedRandomRoots.length(); l++) {
				v *= encryptedRandomRoots[l][j]; 
			}
			VY_cli[0].append(v);

			ZZ_p y = conv<ZZ_p>(0);
			for (int l = 0; l < F[i].length(); l++)
				y += to_ZZ_p(PRF_AES(conv<ZZ_p>(j), F[i][l]));
			VY_cli[1].append(y);
			
			if (j==0) //degbugging
				Y_all[j].append(y);
		}


		// re-encode outsourced puzzle
		Vec<ZZ_p> d_vector;
		for (int j = 0; j < t+2; j++) {
			ZZ_p e = selected_client.q * VY_cli[0][j] * inv(to_ZZ_p(ZW[1][j]));
			ZZ_p e_prime = -(selected_client.q * VY_cli[0][j] * to_ZZ_p(ZW[0][j])) + VY_cli[1][j];
			
			Vec<ZZ> ab;
			ab.append(rep(e_prime));
			ab.append(rep(e));
			Vec<ZZ> su;
			su.append(rep(random_ZZ_p()));
			su.append(rep(random_ZZ_p()));
			ZZ_p d = OLE_p.runOLE_plus(rep(S.o_vectors[i][j]), ab, su, PF);

			ZZ_p d_expected = e * to_ZZ_p(rep(S.o_vectors[i][j])) + e_prime;
			if (d != d_expected) std::cout << "OLE mismatch at (i=" << i << ", j=" << j << ")" << std::endl;

			d_vector.append(d);
		}
		d_vector_nonLeaders.append(d_vector);

		std::cout << "[LinearComb] Computations (non-leader) granted (" << c << "/" << clientsCount - t << ")" << std::endl;
		c++;
	}

};

void LinearCombinations::computeCombination() {
	std::cout << "\n[LinearComb] Combining puzzles" << std::endl;

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

	std::cout << "[LinearComb] Leaders contributed: " << d_vector_leaders.length() << std::endl;
	std::cout << "[LinearComb] Non-leaders contributed: " << d_vector_nonLeaders.length() << std::endl;
};

const Vec<ZZ_p> LinearCombinations::getG_vector() const {return g_vector;};
const Vec<Vec<ZZ>> LinearCombinations::get_tK() const {return tK;};
const Vec<Vec<ZZ>> LinearCombinations::getPP_eval() const {return PP_Eval;};
const std::vector<int> LinearCombinations::get_leaderIndices() const {return selected_leaders;};
const Vec<ZZ_p> LinearCombinations::get_roots() const {return roots;};

const Vec<ZZ_p> LinearCombinations::compute_and_publish() {
	//perform unit tests here most likely
	selectLeaders();
	grantComputations();
	grantComputations_nonLeader();
	computeCombination();
	return getG_vector();
};


