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
		int targetClient; 
		Vec<Vec<ZZ>> PP_eval;
		Vec<Vec<ZZ>> PP;
		Vec<ZZ_p> roots;
		// again, pk_s
		const ZZ p;
		Vec<ZZ_p> X;
		const int t;
		std::vector<int> leader_indices;
		
		// mutables
		// combination
		Vec<Vec<ZZ>> tK; 
		Vec<ZZ_p> theta;
		ZZ_p cons;
		ZZ_p res;
		// single client puzzle	
		Vec<ZZ> K; // single puzzles only
		Vec<ZZ_p> pi;

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
		void o_extract_and_publish();
	public: 
		ZZ_p g_output;
		ZZ_p o_output;
		Vec<Vec<ZZ_p>> g_proof;
		ZZ o_proof;
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
			int targetClient,
			Vec<Vec<ZZ>> PP, 
			Vec<ZZ_p> roots,
			ZZ p, 
			Vec<ZZ_p> X, 
			int t); // single
		void g_solve();
		void o_solve();
};

#endif
