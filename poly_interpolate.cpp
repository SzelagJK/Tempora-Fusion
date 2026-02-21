#include "poly_interpolate.h"

ZZ_p evaluate_and_interpolate(Vec<ZZ_p> X, Vec<ZZ_p> Y, ZZ_p eval_at) {
	ZZ_p P = ZZ_p(0); // evaluation of the interpolated polyonmial P at x 
	for (int i = 0; i < X.length(); i++) {
		ZZ_p L = ZZ_p(1);
		for (int j = 0; j < X.length(); j++) {
			if (j == i)
				continue;
			L *= (eval_at - X[j]) * inv(X[i] - X[j]);
		}
		P += Y[i] * L;
	}
	return P;
};


// experimental, not used in the solution
ZZ_pX interpolate_polynomial(const Vec<ZZ_p>& X, const Vec<ZZ_p>& Y) {
    if (X.length() != Y.length()) {
        LogicError("interpolate_polynomial: X and Y must have same length");
    }
    const long n = X.length();
    if (n == 0) {
        LogicError("interpolate_polynomial: need at least one point");
    }

    ZZ_pX P;
    clear(P);

    for (long i = 0; i < n; ++i) {
        ZZ_pX Ni;
        set(Ni); // Ni = 1

        ZZ_p di = ZZ_p(1);

        for (long j = 0; j < n; ++j) {
            if (j == i) continue;

            // Ni *= (x - X[j])
            ZZ_pX factor;
            clear(factor);
            SetCoeff(factor, 1, ZZ_p(1));
            SetCoeff(factor, 0, -X[j]);
            Ni *= factor;

            // di *= (X[i] - X[j])
            ZZ_p diff = X[i] - X[j];
            if (IsZero(diff)) {
                LogicError("interpolate_polynomial: duplicate X values");
            }
            di *= diff;
        }

        P += (Y[i] * inv(di)) * Ni;
    }

    return P;
}

Vec<ZZ_p> interpolate_roots(const Vec<ZZ_p>& X, const Vec<ZZ_p>& Y) {
    if (X.length() != Y.length()) {
        LogicError("interpolate_roots: X and Y must have the same length");
    }
    const long n = X.length();
    if (n == 0) {
        LogicError("interpolate_roots: need at least one point");
    }

    ZZ_pX P;
    clear(P);

    for (long i = 0; i < n; ++i) {
        ZZ_pX Ni;
        set(Ni); // Ni = 1

        ZZ_p di = ZZ_p(1); // denominator product

        for (long j = 0; j < n; ++j) {
            if (j == i) continue;

            // Ni *= (x - X[j])
            ZZ_pX factor;
            clear(factor);
            SetCoeff(factor, 1, ZZ_p(1));
            SetCoeff(factor, 0, -X[j]);
            Ni *= factor;

            // di *= (X[i] - X[j])
            ZZ_p diff = X[i] - X[j];
            if (IsZero(diff)) {
                LogicError("interpolate_roots: duplicate X values encountered");
            }
            di *= diff;
        }

        P += (Y[i] * inv(di)) * Ni;
    }

    if (IsZero(P)) {
        LogicError("interpolate_roots: interpolated polynomial is identically zero");
    }

    ZZ_p lc = LeadCoeff(P);
    if (IsZero(lc)) {
        LogicError("interpolate_roots: unexpected zero leading coefficient");
    }
    P *= inv(lc);

    vec_pair_ZZ_pX_long fac; // (irreducible factor, exponent)
    CanZass(fac, P);

    Vec<ZZ_p> roots;
    roots.SetLength(0);

    for (long k = 0; k < fac.length(); ++k) {
        const ZZ_pX& g = fac[k].a;

        if (deg(g) == 1) {
            // g(x) = a1*x + a0  =>  root r = -a0/a1
            ZZ_p a1 = coeff(g, 1);
            ZZ_p a0 = coeff(g, 0);

            if (IsZero(a1)) continue; // defensive
            ZZ_p r = -a0 * inv(a1);

            append(roots, r); // distinct because factor list is unique
        }
    }

    return roots;
}
