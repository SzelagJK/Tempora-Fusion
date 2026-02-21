#ifndef GEN_PUZZLE_H
#define GEN_PUZZLE_H

#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <NTL/vec_ZZ_p.h>
#include <vector>
#include <iostream>
#include <cassert>
#include <cmath>

#include "tlp.h"
#include "helper_functions.h"
#include "setup.h"
#include "commitment.h"


struct GenPuzzlesOutput {
	Vec<Vec<ZZ_p>> o_vectors; // all puzzles
	Vec<Vec<ZZ>> PP; // public params of all clients 
};

struct PRMContainer {
	Vec<Vec<ZZ>> SP;
	Vec<Vec<ZZ>> PP;
};

// Class handles generation of multiple puzzles at the same time provided a list of clients, their keys, and corresponding messages
// Note for future self: implement the same outine but for a single puzzle generation (for now this will suffice)
class VHLCTLP_GenPuzzles {
	private:
		// Self-note: use helper functions to encode as ZZ
		const Vec<ZZ> M;
                // Corresponds to all K_u (vector of user keys), use GenerateMultipleClients
		std::vector<Setup_C> K;
		// Corrsponds to pk_s: (p, X, t)
		const ZZ p;
		Vec<ZZ_p> X;
		const int t; // leaders
		const int clientsCount; 
		// delta_u
		std::vector<int> delta;
		int max_ss; // based on the servers max squared sequential operations, ignore individual users
		
		Vec<Vec<ZZ>> secretKeys;
		Vec<ZZ> T; // All delta_u * max_ss
		Vec<ZZ> N; // Clients puzzle public keys
		Vec<ZZ> R; // All r_u bases
		Vec<Vec<Vec<ZZ>>> blindingFactors;
		Vec<Vec<ZZ_p>> encodedMessages;
		Vec<Vec<ZZ_p>> encryptedMessages; // corresponds to "puzzles"
		Vec<ZZ> messageCommitments;
		Vec<Vec<ZZ>> SP; // secret parameters of each client, refer directly to this object to get them separetly
		PRMContainer PRM; // all prm_u = (sp_u, pp_u), use only for clients

		void checkParams() const;
		void generateSecretKeys();
		void generateBlindingFactors();
		void encodeMessages();
	      	void encryptMessages();
		void commitMessages();
	public:
		VHLCTLP_GenPuzzles(
				Vec<ZZ> M, 
				std::vector<Setup_C> K, 
				ZZ p, 
				Vec<ZZ_p> X, 
				int t, 
				int clientsCount, 
				std::vector<int> delta, 
				int max_ss);
		const GenPuzzlesOutput generate_and_publish();
		const PRMContainer getPRMs() const;
};


#endif
