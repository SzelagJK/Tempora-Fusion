#include "gen_puzzle.h"
#include "setup.h"
#include "prf.h"

VHLCTLP_GenPuzzle::VHLCTLP_GenPuzzle(
		Vec<ZZ> M,  
		Vec<Setup_C> K, // Setup_C conatins both pk and sk
		ZZ p, 
		Vec<ZZ_p> X, 
		int t, 
		std::vector<int> delta, 
		int max_ss) : 
	M(std::move(M)), 
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
	for (int i = 0; i < K.length(); i++) {
		Vec<ZZ> key;
		// set exponent (3b-i)
		int T = delta[i] * max_ss;
		ZZ a;
		PowerMod(a, conv<ZZ>(2), conv<ZZ>(T), rep(K[i].getPublicKey()));
		// generate master key (3b-ii)
		ZZ r = RandomBits_ZZ(NumBits(rep(K[i].getSecretKey())));
		ZZ mk;
		PowerMod(mk, r, a, rep(K[i].getSecretKey()));
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
		ZZ z = PRF_AES(conv<ZZ_p>(i), secretKeys[i][1]);
		ZZ w = PRF_AES(conv<ZZ_p>(i), secretKeys[i][2]);
		blindingPair.append(z);
		blindingPair.append(w);

		blindingFactors.append(blindingPair);	
	}
	return blindingFactors;
}

// VHLCTLP: 3d
const Vec<ZZ_p> VHLCTLP_GenPuzzle::encodeMessages() const {
	// intuition: "encoded messages" here refer to the computed y-coordinates for each message m over corresponding root in X using pi(x) 
	Vec<ZZ_p> encodedMessages;
	for (int i = 0; i < M.length(); i++) {
		// pi(x) = x + m mod p where x is an element from X (roots) and p is from sk_s (should be the ZZ_p::modulus()) 
		ZZ_p enc = EncodeAsPoly(X[i], M[i]);
		encodedMessages.append(enc);
	}
	return encodedMessages;
}

// VHLCTLP: 3e
const Vec<ZZ_p> VHLCTLP_GenPuzzle::encrypt(Vec<ZZ_p>& encodings, Vec<Vec<ZZ>>& blindingFactors) const {
	Vec<ZZ_p> encryptedMessages;
	for (int i = 0; i < encodings.length(); i++) {
		ZZ_p o = to_ZZ_p(blindingFactors[i][1] * (rep(encodings[i]) + blindingFactors[i][0]));
		encryptedMessages.append(o);
	}
	return encryptedMessages;
}

// VHLCTLP: 3f
const Vec<ZZ> VHLCTLP_GenPuzzle::commitMessages(Vec<Vec<ZZ>>& secretKeys) const {
	Vec<ZZ> commitments;
	for (int i = 0; M.length(); i++) {
		ZZ comm = commit(M[i], secretKeys[i][0]);
		commitments.append(comm);
	}
	return commitments;
}

// publish

// note for later, move all outputs to private variables, set output to void














