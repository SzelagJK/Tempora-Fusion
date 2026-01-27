#include "OLE.h"
#include <gmp.h>
#include <NTL/vector.h>
#include <NTL/vec_ZZ.h>
#include <NTL/vec_long.h>

using namespace NTL;

OLE_Interface::OLE_Interface(ZZ tau, ZZ log2T) : tau(std::move(tau)), n(log2T + 2 * tau) {}
const ZZ& OLE_Interface::getTau() const {return tau;}
const ZZ& OLE_Interface::getN() const {return n;}

Vec<ZZ_p, ZZ_p> generateR(OLE_Interface& OLE, Poly_Field& PF, Vec<long> alpha) {
	assert(!IsZero(ZZ_p::modulus()));
	ZZ n = OLE.getN(); // The hRP(alpha) in R_{n+1} will be treated separately, to avoid data type conflict 
	Vec<ZZ_p,ZZ_p> R;
	R.SetLength(n);	
	for (int i = 0; i < n-1; i++) {
		// Randomly generate polynomials
		FirstDegPolynomial R_i = FirstDegPolynomial(PF);
		R[i][0] = R_i.P[0];
		R[i][1] = R_i.P[1];
	}	
	return R;
}

Vec<long> generateAlpha(OLE_Interface& OLE) {
	ZZ n = OLE.getN();
	Vec<long> alpha;
	alpha.SetLength(n);

	for (int i = 0; i < n; i++) {
		alpha[i] = RandomBnd(2);
	}
	return alpha;
}

ZZ_p hRP(
		OLE_Interface OLE, 
		FirstDegPolynomial& Poly, 
		Vec<Pair<ZZ_p, ZZ_p>>& R, 
		Vec<long>& alpha) 
{
	ZZ n = OLE.getN();
	ZZ_p h[2];
	h[0] = Poly.P[0];
	h[1] = Poly.P[1];
	for (long i = 0; i < n; i++) {
		if (alpha[i]) {
			h[0] -= R[i].a;
			h[1] -= R[i].b;			
		}
	}
	return h;
}


// Tasks for next time I pick this up:
// Protocol 2, step 1, start implementing (so far got the underlying interface for OLE, now use it for actual protocol)
// continue with the rest
