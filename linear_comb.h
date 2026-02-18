#ifndef LINEAR_COMB_H
#define LINEAR_COMB_H

#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <NTL/vec_ZZ_p.h>
#include <vector>
#include <cassert>
#include <iostream>
#include <algorithm>

#include "setup.h"
#include "gen_puzzle.h"
#include "coin_toss.h"
#include "prf.h"
#include "ole_enhanced.h"

using namespace NTL;

// Intuition: we take the Evaluate() definition from step 4 of the protocol and define each input (param) of it- 
// -Evaluate(s(params), c_1(params), ..., c_n(params)) as S and C structs, keeping the input handling separate from-
// functionality behind linear combinations object. This should look like: Evaluate(S, C_1, ..., C_n) -> (g_dash, pp_eval)
// We further abstract the clients into one vector, so one should expect: Evaluate(S, Cv(C_1, ..., C_n)) -> (g_dash, pp_eval)

// this way it's also more convenient to handle clients and their parameters (given their sheer number and their complexity),
// refer to setup.h and gen_puzzle.h for details for their construction

// for experiment purposes, we are using PRM from VHLCTLP_GenPuzzles.getPRMs(), using corresponding index for i-th clients prm_u
// hence you'll see Evaluate(S, Cv(C_1, ..., C_n), PRMs)

struct S_LinearCombInput {
	Vec<Vec<ZZ_p>> o_vectors;
	int delta_combination; // for now, assume this is delta time of the combination puzzle
	int max_ss;
	Vec<Vec<ZZ>> PP; // coresspond to PP from GenPuzzlesOutput
	Vec<Vec<ZZ>> PK; // seems redundant, check with Aydin
	// pk_s
	ZZ p;
	Vec<ZZ_p> X;
	int t; // self note: still leaders num
};

struct C_LinearCombInput {
	int delta_puzzle;
	int max_ss;
	std::vector<Setup_C> K; // Kept in a vector to avoid initialisation issues 
	int q;
};

class LinearCombinations {
	// Clarificaiton: we use tmp_NAME to define temporary variable *in* a function, and tNAME to define a temporary values specified by the protocol
	private:
		S_LinearCombInput S;
		std::vector<C_LinearCombInput> C_vector;
		PRMContainer PRMs;
		int t;
		OLE_enhanced OLE_p;
		Poly_Field PF;

		int clientsCount;
		std::vector<int> selected_leaders; // here is where coin_toss comes in
		Vec<Vec<ZZ>> tK; // temporary secret keys
		Vec<Vec<ZZ>> F; // random keys f_l meant for each client i, F[leaders][leaders-1]
		Vec<Vec<ZZ>> tBlindingFactors;  
		Vec<ZZ_p> roots;
		Vec<Vec<ZZ_p>> encryptedRandomRoots;
		Vec<Vec<ZZ>> rFactors; // regenerated factors
		Vec<Vec<ZZ_p>> d_vector_leaders;
		Vec<Vec<ZZ_p>> d_vector_nonLeaders;

		Vec<Vec<ZZ>> PP_Eval;
		Vec<ZZ_p> g_vector; // puzzle combination

		void selectLeaders();
                void grantComputations();
                void grantComputations_nonLeader();
		void computeCombination();
	public:
		LinearCombinations(
				S_LinearCombInput S, 
				std::vector<C_LinearCombInput> C_vector, 
				PRMContainer PRMs, 
				int t, 
				int clientsCount, 
				OLE_enhanced OLE_p,
				Poly_Field PF);
		const Vec<ZZ_p> getG_vector() const;
		const Vec<ZZ_p> compute_and_publish();
};

#endif
