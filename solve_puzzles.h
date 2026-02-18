#ifndef SOLVE_PUZZLES_H
#define SOLVE_PUZZLES_H

#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <NTL/vec_ZZ_p.h>
#include <vector>
#include <iostream>

#include "prf.h"

// class used for a single puzzle, not multiple
class SolvePuzzle {
	private:
		const int cmd; // solvePzl or evalPzl (denoted as 0 or !0) (depending on if we are solving a single clients puzzle or a linear combination) 
		Vec<ZZ_p> puzzle_vector; // ^either o or g 
		Vec<Vec<ZZ>> PP_eval;
		Vec<ZZ> pp_u;
		// again, pk_s
		const ZZ p;
		Vec<ZZ_p> X;
		const int t;

		// combination
		void combination_findSecretKeys();
		void combination_removeBlindFactors();
		void combination_extractPolynomial();
		void extractLinearCombination();
		void combination_extractValidRoots();
		void combination_solve_and_publish();
	public: 
		SolvePuzzle(int cmd, Vec<ZZ_p> puzzle_vector, Vec<Vec<ZZ>> PP_eval, ZZ p, Vec<ZZ_p> X, int t); // combination
		SolvePuzzle(int cmd, Vec<ZZ_p> puzzle_vector, Vec<ZZ> pp_u, ZZ p, Vec<ZZ_p> X, int t); // single
};

#endif
