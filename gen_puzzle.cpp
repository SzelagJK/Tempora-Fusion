#include "gen_puzzle.h"
#include "setup.h"
#include "prf.h"

VHLCTLP_GenPuzzle::VHLCTLP_GenPuzzle(
		std::string m, 
		Vec<Setup_C> K, // Setup_C conatins both pk and sk
		ZZ p, 
		Vec<ZZ_p> X, 
		int t, 
		std::vector<int> delta, 
		int max_ss) : 
	m(std::move(m)), 
	K(std::move(K)), 
	p(std::move(p)), 
	X(std::move(X)), 
	t(std::move(t)), 
	delta(std::move(delta)), 
	max_ss(std::move(max_ss)) {}

// VHLCTLP: 3a
const void VHLCTLP_GenPuzzle::checkParams() const {
	if (NumBits(p) < 128)
		throw std::invalid_argument("Prime too small for secure puzzle generation.");
	
	for (int i = 0; i < X.length(); i++) {
		if (X[i] == 0)
			throw std::invalid_argument("X contains zero entries.");
	}
};

// VHLCTLP: 3b
const Vec<Vec<ZZ>> VHLCTLP_GenPuzzle::generateSecretKeys() const {
	Vec<Vec<ZZ>> keys;
	for (int i = 0; i < K.size(); i++) {
		Vec<ZZ_p> key;
		// set exponent (3b-i)
		int T = delta[i] * max_ss;
		ZZ a;
		PowerMod(a, 2, T, K[i].getPublicKey());
		// generate master key (3b-ii)
		ZZ r = RandomBits_ZZ(K[i].getSecretKey());
		ZZ mk;
		PowerMod(mk, r, a, K[i].getSecretKey());
		key.append(mk);
		// derive k and s (3b-iii)
		ZZ k = PRF_AES(conv<ZZ_p>(1), mk); // consider changing the x param datatype to ZZ later  
		ZZ s = PRF_AES(conv<ZZ_p>(2), mk);
		key.append(k);
		key.append(s);

		keys.append(key);
	}
	return keys;
}

// VHLCTLP: 3c
const Vec<Vec<ZZ>> VHLCTLP_GenPuzzle::generateBlindingFactors(Vec<Vec<ZZ>>& secretKeys) const {
	Vec<Vec<ZZ>> blindingFactors;
	for (int i = 0; i < secretKeys.length(); i++) {
		Vec<ZZ> blindingPair;
		ZZ z = AES_PRF(conv<ZZ_p>(i), secretKeys[i][0]);
		ZZ w = AES_PRF(conv<ZZ_p>(i), secretKeys[i][1]);
		blindingPair.append(z);
		blindingPair.append(w);

		blindingFactors.append(blindingPair);	
	}
	return blindingFactors;
}







