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
	for (int i = 0; i < t; i++) {
		leader_client = C_vector[selected_leaders[i]];
		ZZ b;
		ZZ Y = conv<ZZ>(leader_client.delta_puzzle * leader_client.max_ss);
		PowerMod(b, conv<ZZ>(2), Y, conv<ZZ>(leader_client.K.getSecretKey()));
		
		ZZ n = leader_client.K.getPublicKey();
		ZZ h;
		do {
			h = RandomBits_ZZ(NumBits(n));
			h %= n;
		} while (GCD(h, n) != 1);

		ZZ tk;
		PowerMod(tk, h, b, conv<ZZ>(n));

		ZZ k_prime = PRF_AES(conv<ZZ>(1), tk);
		ZZ s_prime = PRF_AES(conv<ZZ>(2), tk);

		// okay so how do i do this bullshit

	}
}
