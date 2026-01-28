#ifndef OLE_H
#define OLE_H

#include <NTL/ZZ.h>
#include <gmp.h>
#include <gmpxx.h>
#include <NTL/ZZ_p.h>
#include <NTL/vec_ZZ_p.h>
#include <NTL/vec_long.h>

#include "poly_field.h"

using namespace NTL;

class OLE_Interface {
	private:
		const ZZ tau, n;
	public:
		OLE_Interface(ZZ tau, ZZ log2T);
		const ZZ& getTau() const;
		const ZZ& getN() const;
};

// Sidenote: For code clarity, I could consider putting those functions in the OLE interface object 
// (so that they don't need OLE interface being passed constantly)
Vec<Vec<ZZ_p>> generateR(OLE_Interface& OLE, Poly_Field& PF, Vec<long>& alpha);
Vec<long> generateAlpha(OLE_Interface& OLE);
Vec<ZZ_p> hRP(OLE_Interface& OLE, FirstDegPolynomial& Poly, Vec<Vec<ZZ_p>>& R, Vec<long>& alpha);

#endif
