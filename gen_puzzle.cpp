#include "gen_puzzle.h"
#include "setup.h"
#include "prf.h"

VHLCTLP_GenPuzzles::VHLCTLP_GenPuzzles(
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
void VHLCTLP_GenPuzzles::checkParams() const {
	if (NumBits(p) < 128)
		throw std::invalid_argument("Prime too small for secure puzzle generation.");
	
	for (int i = 0; i < X.length(); i++) {
		if (X[i] == 0)
			throw std::invalid_argument("X contains zero entries.");
	}
};

// VHLCTLP: 3b
void VHLCTLP_GenPuzzles::generateSecretKeys() {
	Vec<Vec<ZZ>> keys;
	Vec<ZZ> tmp_T;
	Vec<ZZ> tmp_N;
	Vec<ZZ> tmp_R;
	Vec<Vec<ZZ>> tmp_SP;
	for (int i = 0; i < K.length(); i++) {
		Vec<ZZ> key;
		Vec<ZZ> sp_u; // for separate containment of SP
		// set exponent (3b-i)
		int T_u = delta[i] * max_ss;
		tmp_T.append(conv<ZZ>(T_u));
		ZZ a;
		PowerMod(a, conv<ZZ>(2), conv<ZZ>(T_u), rep(K[i].getSecretKey()));
		// generate master key (3b-ii)
		ZZ n = rep(K[i].getPublicKey());
		ZZ r = RandomBits_ZZ(NumBits(n));
		tmp_N.append(n);
		tmp_R.append(r);
		ZZ mk;
		PowerMod(mk, r, a, rep(K[i].getPublicKey()));
		key.append(mk);
		// derive k and s (3b-iii)
		ZZ k = PRF_AES(conv<ZZ_p>(1), mk); // consider changing the x param datatype to ZZ later  
		ZZ s = PRF_AES(conv<ZZ_p>(2), mk);
		key.append(k);
		sp_u.append(k);
		key.append(s);
		sp_u.append(s);

		keys.append(key);
		tmp_SP.append(sp_u);
	}
	T = tmp_T;
	N = tmp_N;
	R = tmp_R;
	secretKeys = keys;
}

// VHLCTLP: 3c
void VHLCTLP_GenPuzzles::generateBlindingFactors() {
	Vec<Vec<ZZ>> tmp_blindingFactors;
	// we use private secretKeys Vec object
	for (int i = 0; i < secretKeys.length(); i++) {
		Vec<ZZ> blindingPair;
		ZZ z = PRF_AES(conv<ZZ_p>(i), secretKeys[i][1]);
		ZZ w = PRF_AES(conv<ZZ_p>(i), secretKeys[i][2]);
		blindingPair.append(z);
		blindingPair.append(w);

		tmp_blindingFactors.append(blindingPair);	
	}
	blindingFactors = tmp_blindingFactors;
}

// VHLCTLP: 3d
void VHLCTLP_GenPuzzles::encodeMessages() {
	// intuition: "encoded messages" here refer to the computed y-coordinates for each message m over corresponding root in X using pi(x) 
	Vec<ZZ_p> tmp_encodedMessages;
	for (int i = 0; i < M.length(); i++) {
		// pi(x) = x + m mod p where x is an element from X (roots) and p is from sk_s (should be the ZZ_p::modulus()) 
		ZZ_p enc = EncodeAsPoly(X[i], M[i]);
		tmp_encodedMessages.append(enc);
	}
	encodedMessages = tmp_encodedMessages;
}

// VHLCTLP: 3e
void VHLCTLP_GenPuzzles::encryptMessages() {
	Vec<ZZ_p> tmp_encryptedMessages;
	for (int i = 0; i < encodedMessages.length(); i++) {
		ZZ_p o = to_ZZ_p(blindingFactors[i][1] * (rep(encodedMessages[i]) + blindingFactors[i][0]));
		tmp_encryptedMessages.append(o);
	}
	encryptedMessages = tmp_encryptedMessages;
}

// VHLCTLP: 3f
void VHLCTLP_GenPuzzles::commitMessages() {
	Vec<ZZ> tmp_commitments;
	for (int i = 0; M.length(); i++) {
		ZZ comm = commit(M[i], secretKeys[i][0]);
		tmp_commitments.append(comm);
	}
	messageCommitments = tmp_commitments;
}

// VHLCTLP: 3g
// Repesent all ZZ_p as ZZ for containment puposes
const GenPuzzlesOutput VHLCTLP_GenPuzzles::generate_and_publish() const {
	GenPuzzlesOutput out;
	
	checkParams();
	generateSecretKeys();
	geneateBlindingFactors();
	encodeMessages();
	encryptMessages();
	commitMessages();

	out.o_vector = encryptedMessages;

	tmp_PP.append(messageCommitments);
	tmp_PP.append(T);
	tmp_PP.append(R);
	tmp_PP.append(N);
	out.PP = tmp_PP;

	// set PRM
	PRMContainer tmp_PRM;
	tmp_PRM.SP = SP;
	tmp_PRM.PP = tmp_PP;

	return out;
}

// note for later, move all outputs to private variables, set output to void














