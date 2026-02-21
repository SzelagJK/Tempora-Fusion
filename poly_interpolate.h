#ifndef INTERPOLATE_H
#define INTERPOLATE_H

#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <NTL/vec_ZZ_p.h>
#include <NTL/ZZ_pX.h>
#include <NTL/ZZ_pXFactoring.h>
#include <vector>
#include <iostream>
#include <set>
#include <cassert>

using namespace NTL;

// assumes constant term is 0, hence set as default, Step 5, part c
ZZ_p evaluate_and_interpolate(Vec<ZZ_p> X, Vec<ZZ_p> Y, ZZ_p eval_at = ZZ_p(0));

// experimental, not used
ZZ_pX interpolate_polynomial(const Vec<ZZ_p>& X, const Vec<ZZ_p>& Y);

Vec<ZZ_p> interpolate_roots(const Vec<ZZ_p>& X, const Vec<ZZ_p>& Y);

#endif
