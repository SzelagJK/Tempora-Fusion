#ifndef GEN_PUZZLE_H
#define GEN_PUZZLE_H

#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <NTL/vec_ZZ_p.h>
#include <vector>
#include <iostream>
#include <cassert>

#include "tlp.h"
#include "helper_functions.h"
#include "setup.h"
#include "commitment.h"

// outputs a vector of puzzles
class VHLCTLP_GenPuzzle {
	private:
		// Self-note: use helper functions to encode as ZZ
		const Vec<ZZ> M;
                // Corresponds to K_u (vector of user keys), use GenerateMultipleClients
                Vec<Setup_C> K;
		// Corrsponds to pk_s: (p, X, t)
		const ZZ p;
		Vec<ZZ_p> X;
		const int t;
		// delta_u
		std::vector<int> delta;
		int max_ss; // based on the servers max squared sequential operations, ignore individual users
	public:
		VHLCTLP_GenPuzzle(Vec<ZZ> M, Vec<Setup_C> K, ZZ p, Vec<ZZ_p> X, int t, std::vector<int> delta, int max_ss);
		const void checkParams() const;
		// All users generate 3 keys, all under ZZ_p (Step 3b)
		const Vec<Vec<ZZ>> generateSecretKeys() const;
		const Vec<Vec<ZZ>> generateBlindingFactors(Vec<Vec<ZZ>>& secretKeys) const;
		// self-note for later: could be poly_field
		const Vec<ZZ_p> encodeMessages() const;
	      	const Vec<ZZ_p> encrypt(Vec<ZZ_p>& encodings, Vec<Vec<ZZ>>& blindingFactors) const;
		// Most likely a hash since its a committment, so might not be ZZ_p, I'll see
		const Vec<ZZ> commitMessages(Vec<Vec<ZZ>>& secretKeys) const;
	       	// Re-think how publish() should be implemented (dependent on commit), for now this is a placeholder	
		const Vec<ZZ_p> publish() const;
};

#endif
