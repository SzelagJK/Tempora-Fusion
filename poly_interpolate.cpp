#include "poly_interpolate.h"

ZZ_p interpolate_polynomial(Vec<ZZ_p> X, Vec<ZZ_p> Y, ZZ_p eval_at) {
	ZZ_p P = ZZ_p(0); // evaluation of the interpolated polyonmial P at x 
	for (int i = 0; i < X.length(); i++) {
		ZZ_p L = ZZ_p(1);
		for (int j = 0; j < X.length(); j++) {
			if (j == i)
				continue;
			L *= (eval_at - X[j])/(X[i] - X[j]);
		}
		P += Y[i] * L;
	}
	return P;
};
