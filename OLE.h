#ifndef OLE_H
#define OLE_H

#include <NTL/ZZ.h>
#include <gmp.h>
#include <gmpxx.h>
#include <NTL/ZZ_p.h>
#include <NTL/vec_ZZ_p.h>
#include <NTL/vec_long.h>

#include "poly_field.h"
#include "OT_1of2.h"

using namespace NTL;

class OLE_Interface {
	private:
		const ZZ tau, n;
	public:
		OLE_Interface(ZZ tau, int log2T);
		const ZZ& getTau() const;
		const ZZ& getN() const;
		const ZZ_p runOT_and_sum(Vec<Vec<ZZ_p>>& pairs, Vec<long>& alpha, int key_bits) const;
		const ZZ_p extract_eval(ZZ_p& maskedOutput, Vec<ZZ_p>& r) const;
};

// Sidenote: For code clarity, I could consider putting those functions in the OLE interface object 
// (so that they don't need OLE interface being passed constantly)
Vec<Vec<ZZ_p>> generateR(OLE_Interface& OLE, Poly_Field& PF, Vec<long>& alpha); // Set of random polynomials
Vec<long> generateAlpha(OLE_Interface& OLE);
Vec<ZZ_p> hRP(OLE_Interface& OLE, FirstDegPolynomial& Poly, Vec<Vec<ZZ_p>>& R, Vec<long>& alpha);
Vec<ZZ_p> generateVector_r(OLE_Interface& OLE);
Vec<Vec<ZZ_p>> PreparePairs(Vec<ZZ_p> r, Vec<Vec<ZZ_p>> R, ZZ_p x_star);

#endif
