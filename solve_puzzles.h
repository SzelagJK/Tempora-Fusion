#ifndef SOLVE_PUZZLES_H
#define SOLVE_PUZZLES_H

#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <NTL/vec_ZZ_p.h>
#include <NTL/ZZ_pX.h>
#include <vector>
#include <iostream>
#include <cassert>

#include "prf.h"

// class used for a single puzzle, not multiple
class SolvePuzzle {
	private:
		const int cmd; // solvePzl or evalPzl (denoted as 0 or !0) (depending on if we are solving a single clients puzzle or a linear combination) 
		Vec<ZZ_p> puzzle_vector; // ^either o or g 
		Vec<Vec<ZZ>> PP_eval;
		Vec<Vec<ZZ>> PP;
		// again, pk_s
		const ZZ p;
		Vec<ZZ_p> X;
		const int t;

		// mutables
		Vec<Vec<ZZ>> tK; 
		Vec<ZZ_p> theta;

		// combination
		void g_findSecretKeys();
		void g_removeBlindFactors();
		void g_extractPolynomial();
		void g_extractLinearCombination();
		void g_extractValidRoots();
		void g_solve_and_publish();
		// single client puzzle
		void o_findSecretKeys();
		void o_removeBlindFactors();
		void o_exractPolynomial();
		void o_solve_and_publish();
	public: 
		SolvePuzzle(int cmd, Vec<ZZ_p> puzzle_vector, Vec<Vec<ZZ>> PP_eval, ZZ p, Vec<ZZ_p> X, int t); // combination
		SolvePuzzle(int cmd, Vec<ZZ_p> puzzle_vector, Vec<ZZ> pp_u, ZZ p, Vec<ZZ_p> X, int t); // single
};

#endif
