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
#include "poly_interpolate.h"
#include "commitment.h"

// class used for a single puzzle, not multiple
class SolvePuzzle {
	private:
		const int cmd; // solvePzl or evalPzl (denoted as 0 or !0) (depending on if we are solving a single clients puzzle or a linear combination) 
		Vec<ZZ_p> puzzle_vector; // ^either o or g 
		Vec<Vec<ZZ>> PP_eval;
		Vec<Vec<ZZ>> PP;
		Vec<ZZ> pp_u;
		Vec<ZZ_p> roots;
		// again, pk_s
		const ZZ p;
		Vec<ZZ_p> X;
		const int t;
		std::vector<int> leader_indices;
		
		// mutables
		Vec<Vec<ZZ>> tK; 
		Vec<ZZ_p> theta;
		ZZ_p cons;
		ZZ_p res;

		// combination
		void g_findSecretKeys();
		void g_removeBlindFactors();
		void g_extractPolynomial();
		void g_extractLinearCombination();
		void g_extractValidRoots();
		void g_publish();
		// single client puzzle
		void o_findSecretKeys();
		void o_removeBlindFactors();
		void o_exractPolynomial();
		Vec<Vec<ZZ_p>> o_solve_and_publish();
	public: 
		ZZ_p g_output;
		Vec<Vec<ZZ_p>> proof;
		SolvePuzzle(
			int cmd, 
			Vec<ZZ_p> puzzle_vector, 
			Vec<Vec<ZZ>> PP_eval, 
			Vec<Vec<ZZ>> PP,
			Vec<ZZ_p> roots,
			ZZ p, 
			Vec<ZZ_p> X, 
			int t, 
			std::vector<int> leader_indices); // combination
		SolvePuzzle(
			int cmd, 
			Vec<ZZ_p> puzzle_vector, 
			Vec<Vec<ZZ>> PP, 
			Vec<ZZ> pp_u, 
			Vec<ZZ_p> roots,
			ZZ p, 
			Vec<ZZ_p> X, 
			int t, 
			std::vector<int> leader_indices); // single
		void g_solve();
};

#endif
