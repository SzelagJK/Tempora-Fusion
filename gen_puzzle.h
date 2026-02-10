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

// outputs a vector of puzzles
class VHLCTLP_GenPuzzle {
	private:
		// Self-note: use helper functions to convert to ZZ
		const std::string m;
                // Corresponds to K_u (vector of public user keys)
                Vec<ZZ_p> K;
		// Corrsponds to pk_s: (p, X, t)
		const ZZ p;
		Vec<ZZ_p> X;
		const int t;
		// delta_u
		std::vector<int> delta;
		int max_ss; // based on the servers max squared sequential operations, ignore individual users
	public:
		VHLCTLP_GenPuzzle(std::string m, Vec<ZZ_p> K, ZZ p, Vec<ZZ_p> X, int t, std::vector<int> delta, int max_ss);
		const void checkParams() const;
		// All users generate 3 keys, all under ZZ_p (Step 3b)
		const Vec<Vec<ZZ_p>> generateSecretKeys() const;
		const Vec<ZZ_p> generateBlindingFactors() const;
		// self-note for later: could be poly_field
		const ZZ_p encodeAsPoly() const;
	      	const ZZ_p encrypt() const;
		// Most likely a hash since its a committment, so might not be ZZ_p, I'll see
		const ZZ_p commit() const;
	       	// Re-think how publish() should be implemented (dependent on commit), for now this is a placeholder	
		const ZZ_p publish() const;
};

#endif
