#ifndef LINEAR_COMB_H
#define LINEAR_COMB_H

#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <NTL/vec_ZZ_p.h>
#include <vector>
#include <cassert>
#include <iostream>

#include "setup.h"
#include "gen_puzzle.h"
#include "coin_toss.h"
#include "prf.h"

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
	Vec<Vec<ZZ_p>> o_vectos;
	const int delta_combination; // for now, assume this is delta time of the combination puzzle
	const int max_ss;
	Vec<Vec<ZZ>> PP; // coresspond to PP from GenPuzzlesOutput
	Vec<Vec<ZZ>> PK; // seems redundant, check with Aydin
	// pk_s
	const ZZ p;
	Vec<ZZ_p> X;
	const int t; // self note: still leaders num
};

struct C_LinearCombInput {
	int delta_puzzle;
	int max_ss;
	std::vector<Setup_C> K; 
};

class LinearCombinations {
	private:
		S_LinearCombInput S;
		std::vector<C_LinearCombInput> C_vector;
		PRMContainer PRMs;
		int t;
		int clientsCount;
		std::vector<int> selected_leaders; // here is where coin_toss comes in
		Vec<Vec<ZZ>> tK; // temporary secret keys
	public:
		LinearCombinations(S_LinearCombInput S, std::vector<C_LinearCombInput> C_vector, PRMContainer PRMs, int t);
		void selectLeaders();
		void grantComputations();
};




#endif
