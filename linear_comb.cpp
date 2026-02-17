#include "linear_comb.h"

LinearCombinations::LinearCombinations(
		S_LinearCombInput S, 
		std::vector<C_LinearCombInput> C_vector, 
		PRMContainer PRMs,
		int t) : 
	S(std::move(S)), 
	C_vector(std::move(C_vector)), 
	PRMs(std::move(PRMs)) {
		clientsCount = C_vector.size();
	};

void LinearCombinations::selectLeaders() {
	ZZ r_hat = coinToss(clientsCount);
	std::vector<int> tmp_leaders = determineLeaderIndices(t, clientsCount, r_hat);	
	selected_leaders = tmp_leaders;
}

void LinearCombinations::grantComputations() {
	// generates temporary secret keys
	Vec<Vec<ZZ>> tmp_tK;
	Vec<Vec<ZZ>> tmp_tBlindingFactors;
	Vec<Vec<ZZ_p>> tmp_encryptedRandomRoots;
	Vec<Vec<ZZ>> tmp_BlindingFactors;
	// f_l keys meant for other leaders
	Vec<Vec<ZZ>> tmp_F;
	tmp_F.SetLength(t);
	for (int i = 0; i < t; i++) {
		Vec<ZZ> F_u;
		F_u.SetLength(t-1);
		tmp_F.append(F_u);
	}

	for (int i = 0; i < t; i++) {
		C_LinearCombInput leader_client = C_vector[selected_leaders[i]];

		Vec<ZZ> tK_u;
		ZZ b;
		ZZ Y = conv<ZZ>(leader_client.delta_puzzle * leader_client.max_ss);
		PowerMod(b, conv<ZZ>(2), Y, rep(leader_client.K.getSecretKey()));
		
		ZZ n = rep(leader_client.K.getPublicKey());
		ZZ h;
		do {
			h = RandomBits_ZZ(NumBits(n));
			h %= n;
		} while (GCD(h, n) != 1);

		ZZ tk;
		PowerMod(tk, h, b, conv<ZZ>(n));

		ZZ k_prime = PRF_AES(conv<ZZ_p>(1), tk);
		ZZ s_prime = PRF_AES(conv<ZZ_p>(2), tk);
		tK_u.append(tk);
		tK_u.append(k_prime);
		tK_u.append(s_prime);

		// Each client generates a key f_l for every other client (outputting t-1 keys)
		// append 1 key to every vector of F to simulate broadcast of every key to each client
		long lambda = NumBits(S.p);
		long poly_lambda = lambda * lambda; // hard-coded adversarial computational power 
		for (int j = 0; j < t; j++) {
			if (j == i)
				continue; // skip its own index
			ZZ f_l;
			RandomBits(f_l, poly_lambda);
			tmp_F[j][i] = f_l;
		}

		tmp_tK.append(tK_u);

		// Generate temporary blinding factors
		Vec<ZZ> tfactors;
		for (int j = 1; j <= t+2; i++) {
			ZZ w_prime = PRF_AES(conv<ZZ_p>(j), s_prime);
			tfactors.append(w_prime);
		}
		tmp_tBlindingFactors.append(tfactors);

		// Encrypt random root
		ZZ_p root = random_ZZ_p();
		Vec<ZZ_p> gamma_prime_vector;
		for (int j = 0; j < t+2; j++) {
			ZZ_p gamma = S.X[j] - root; // encodes
			ZZ_p gamma_prime = gamma * to_ZZ_p(tfactors[j]); // encrypts
			gamma_prime_vector.append(gamma_prime);
		}
		tmp_encryptedRandomRoots.append(gamma_prime_vector);

		// Generate blinding factors
		// regenerate previous factors
		Vec<ZZ> factors;
		for (int j = 0; j < t+2; j++) {
			ZZ z = PRF_AES(conv<ZZ_p>(j), PRMs.SP[selected_leaders[i]][0]);
			ZZ w = PRF_AES(conv<ZZ_p>(j), PRMs.SP[selected_leaders[i]][1]);
			factors.append(z);
			factors.append(w);
			ZZ z_prime = PRF_AES(conv<ZZ_p>(j), k_prime);
			factors.append(z_prime); // additional factor
		}
		// set values v and y
		Vec<ZZ> vy_factors;
		for (int j = 0; j < t+2; j++) {
			// work out v_i_u
			ZZ_p product_gamma_prime = conv<ZZ_p>(1);
			for (int l = 0; l < t+2; l++) {
				if (l == j)
					continue;
				product_gamma_prime *= tmp_encryptedRandomRoots[l][j]; // go around all other leaders encrypted roots, grab i-th (or j-th) element
			}
			ZZ_p v = gamma_prime_vector[j] * product_gamma_prime;
			vy_factors.append(rep(v));

			// mod p comes after, if wrong check again
			ZZ sum_f = conv<ZZ>(0);
			for (int l = 0; l < clientsCount; l++) {
				if (l == i)
					continue; // skip c_u, although here we dont work with indexes so technically doesn't matter if we set condition to clinttsCount - 1
				ZZ f_l;
				RandomBits(f_l, poly_lambda);

				sum_f += PRF_AES(conv<ZZ_p>(l), f_l);
			}
			// second sum, goes over every f_l recieved by other leader clients, and iterates over them t+2 times generating a PRF value and summing it
			ZZ_p sum_f_dash = conv<ZZ_p>(0);
			for (int l = 0; l < t+2; l++) {
				for (int k = 0; k < t-1; k++) {
					sum_f_dash += to_ZZ_p(PRF_AES(conv<ZZ_p>(l), tmp_F[i][k]));
				}
			}

			ZZ_p y = to_ZZ_p(conv<ZZ>(-1) * sum_f) + sum_f_dash;

			vy_factors.append(rep(y));
		}


		

	}

	tK = tmp_tK;
	F = tmp_F;
	encryptedRandomRoots = tmp_encryptedRandomRoots;
}
