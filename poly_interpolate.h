#ifndef INTERPOLATE_H
#define INTERPOLATE_H

#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <NTL/vec_ZZ_p.h>
#include <vector>
#include <iostream>
#include <cassert>

using namespace NTL;

// assumes constant term is 0, hence set as default, Step 5, part c
ZZ_p interpolate_polynomial(Vec<ZZ_p> X, Vec<ZZ_p> Y, ZZ_p eval_at = ZZ_p(1000));


#endif
