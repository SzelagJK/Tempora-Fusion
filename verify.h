#ifndef VERIFY_H
#define VERIFY_H

#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <NTL/vec_ZZ_p.h>
#include <NTL/tools.h>
#include <iostream>
#include <cassert>
#include <string>

#include "prf.h"
#include "commitment.h"
#include "poly_interpolate.h"

using namespace NTL;

class Verify {
	private:
		int cmd;
		ZZ_p m; // decrypted combination or original m_u (depends on the variant used)
		Vec<Vec<ZZ>> g_proof;
		ZZ o_proof;
		Vec<ZZ_p> puzzle_vector;
		Vec<Vec<ZZ>> PP_eval;
		Vec<Vec<ZZ>> PP;
		ZZ p;
		Vec<ZZ_p> X;
		int t;

		// mutable
		bool flag = true;	
		
		void g_checkCommitments();
		void g_checkRoots_and_Result();
		void g_decide();
		
		void o_checkCommitments();
		void o_decide();
	public:
		Verify(
			int cmd, 
			ZZ_p m, 
			Vec<Vec<ZZ>> g_proof, 
			Vec<ZZ_p> puzzle_vector, 
			Vec<Vec<ZZ>> PP_eval, 
			Vec<Vec<ZZ>> PP, 
			ZZ p, 
			Vec<ZZ_p> X, 
			int t); // combination
		Verify(
			int cmd,
			ZZ_p m,
			ZZ o_proof,
			Vec<ZZ_p> puzzle_vector,
			Vec<Vec<ZZ>> PP); // single
		int g_verify();
		int o_verify();
};




#endif
