#ifndef OLE_H
#define OLE_H

#include <NTL/ZZ.h>
#include <gmp.h>
#include <NTL/ZZ_p.h>
#include <NTL/vec_ZZ.h>
#include <NTL/vec_ZZ_p.h>

#include "ope_interface.h"
#include "poly_field.h"

using namespace NTL;

class OLE {
	private:
		const ZZ p;
		const int key_bits;
		Poly_Field PF;
	public:
		OLE(ZZ p, int key_bits, Poly_Field PF);
		const ZZ_p runOLE(ZZ x_star, const Vec<ZZ>& coeff_ab = Vec<ZZ>());
		const ZZ_p runOLE(ZZ_p x_star, const Vec<ZZ>& coeff_ab = Vec<ZZ>());
		const ZZ_p runOLE(ZZ_p x_star, const Vec<ZZ_p>& coeff_ab = Vec<ZZ_p>());
};

template<typename T>
Vec<ZZ> init_coeff_vector(T a, T b) {
	Vec<ZZ> coeffs_ab;
	coeffs_ab.SetLength(2);
	coeffs_ab[0] = conv<ZZ>(a);
	coeffs_ab[1] = conv<ZZ>(b);
	return coeffs_ab;
};

#endif
