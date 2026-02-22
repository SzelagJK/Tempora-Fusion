#include "gen_puzzle.h"
#include "setup.h"
#include "prf.h"

VHLCTLP_GenPuzzles::VHLCTLP_GenPuzzles( // add lambda for puzzles keys prf
		Vec<ZZ> M,  
		std::vector<Setup_C> K, 
		ZZ p, 
		Vec<ZZ_p> X, 
		int t, 
		int clientsCount,
		std::vector<int> delta, 
		int max_ss) : 
	M(std::move(M)), 
	K(std::move(K)), 
	p(std::move(p)), 
	X(std::move(X)), 
	t(std::move(t)), 
	clientsCount(std::move(clientsCount)),
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
	std::cout << "\n[GenPuzzles] Generating Secret Keys" << std::endl;
	Vec<Vec<ZZ>> keys;
	Vec<ZZ> tmp_T;
	Vec<ZZ> tmp_N;
	Vec<ZZ> tmp_R;
	Vec<Vec<ZZ>> tmp_SP;
	for (int i = 0; i < K.size(); i++) {
		Vec<ZZ> key;
		Vec<ZZ> sp_u; // for separate containment of SP
		// set exponent (3b-i)
		int T_u = delta[i] * max_ss;
		tmp_T.append(conv<ZZ>(T_u));
		ZZ a;
		PowerMod(a, conv<ZZ>(2), conv<ZZ>(T_u), K[i].getSecretKey());
		// generate master key (3b-ii)
		ZZ n = K[i].getPublicKey();
		ZZ r;
		do {
			r = RandomBits_ZZ(NumBits(n));
			r %= n;
		} while (GCD(r, n) != 1); // ensuring that r is reducable, later check if can be optimised with how r is randomised
		tmp_N.append(n);
		tmp_R.append(r);
		ZZ mk;
		PowerMod(mk, r, a, n);
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
	SP = tmp_SP;
	secretKeys = keys;
}

// VHLCTLP: 3c
// generates blinding factors for all clients 
void VHLCTLP_GenPuzzles::generateBlindingFactors() {
	std::cout << "[GenPuzzles] Generating Blinding Factors" << std::endl;

	Vec<Vec<Vec<ZZ>>> tmp;
  	tmp.SetLength(clientsCount);

  	for (int c = 0; c < clientsCount; c++) {
    		Vec<Vec<ZZ>> per_client;
    		per_client.SetLength(t + 2);

    		for (int j = 0; j < t + 2; j++) {
      			Vec<ZZ> zw;
      			ZZ z = PRF_AES(conv<ZZ_p>(j), secretKeys[c][1]); // k_c
      			ZZ w = PRF_AES(conv<ZZ_p>(j), secretKeys[c][2]); // s_c
      			zw.append(z);
      			zw.append(w);
      			per_client[j] = zw;
    		}	
    	tmp[c] = per_client;
  	}

  	blindingFactors = tmp;
}

// VHLCTLP: 3d
void VHLCTLP_GenPuzzles::encodeMessages() {
	std::cout << "[GenPuzzles] Encoding messages as polynomials" << std::endl;
	// intuition: "encoded messages" here refer to the computed y-coordinates for each message m over coordinates of X (t_dash y-coordinates to be precise) 
	Vec<Vec<ZZ_p>> tmp_encodedMessages;
	for (int i = 0; i < M.length(); i++) {
		Vec<ZZ_p> enc;
		// pi(x) = x + m mod p where x is an element from X (roots) and p is from sk_s (should be the ZZ_p::modulus()) 
		for (int j = 0; j < t+2; j++) {
			ZZ_p phi_i = EncodeAsPoly(X[j], M[i]);
			enc.append(phi_i);
		}
		tmp_encodedMessages.append(enc);
	}
	encodedMessages = tmp_encodedMessages;
}

// VHLCTLP: 3e
void VHLCTLP_GenPuzzles::encryptMessages() {
  	std::cout << "[GenPuzzles] Encrypting messages" << std::endl;

  	Vec<Vec<ZZ_p>> tmp_encryptedMessages;

  	for (int c = 0; c < encodedMessages.length(); c++) {
    		Vec<ZZ_p> o_vector;

    		for (int j = 0; j < encodedMessages[c].length(); j++) {
      			const ZZ& z = blindingFactors[c][j][0];
      			const ZZ& w = blindingFactors[c][j][1];

      			ZZ_p o = to_ZZ_p(w * (rep(encodedMessages[c][j]) + z));
      			o_vector.append(o);
    		}

    		tmp_encryptedMessages.append(o_vector);
  	}

  	encryptedMessages = tmp_encryptedMessages;
}

// VHLCTLP: 3f
void VHLCTLP_GenPuzzles::commitMessages() {
	std::cout << "[GenPuzzles] Commiting messages" << std::endl;
	Vec<ZZ> tmp_commitments;
	for (int i = 0; i < M.length(); i++) {
		ZZ comm = commit(M[i], secretKeys[i][0]);
		tmp_commitments.append(comm);
	}
	messageCommitments = tmp_commitments;
}

// VHLCTLP: 3g
// Repesent all ZZ_p as ZZ for containment puposes
const GenPuzzlesOutput VHLCTLP_GenPuzzles::generate_and_publish() {
	GenPuzzlesOutput out;
	Vec<Vec<ZZ>> tmp_PP;
	
	checkParams();
	generateSecretKeys();
	generateBlindingFactors();
	encodeMessages();
	encryptMessages();
	commitMessages();

	out.o_vectors = encryptedMessages;

	tmp_PP.append(messageCommitments);
	tmp_PP.append(T);
	tmp_PP.append(R);
	tmp_PP.append(N);
	out.PP = tmp_PP;

	// set PRM
	// do something with it later
	PRMContainer tmp_PRM;
	tmp_PRM.SP = SP;
	tmp_PRM.PP = tmp_PP;
	PRM = tmp_PRM; // Keep in mind that this is a container of vectors, so reach for a corresponding index for i-th clients prm_u

	std::cout << "[GenPuzzles] Puzzle generation complete" << std::endl;

	return out;
}

const PRMContainer VHLCTLP_GenPuzzles::getPRMs() const {return PRM;};

// note for later, move all outputs to private variables, set output to void














