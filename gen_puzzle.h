#ifndef GEN_PUZZLE_H
#define GEN_PUZZLE_H

#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <NTL/vec_ZZ_p.h>
#include <vector>
#include <iostream>
#include <cassert>

#include "tlp.h"

class VHLCTLP_GenPuzzle {
	const std::string m;
	// pk_s: (p, X, t)
	const ZZ p;
	Vec<ZZ_p> X;
	const int t;
};

#endif
