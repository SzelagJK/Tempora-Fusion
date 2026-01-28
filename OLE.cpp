#include "OLE.h"
#include <cassert>

using namespace NTL;

OLE_Interface::OLE_Interface(ZZ tau, ZZ log2T) : tau(std::move(tau)), n(log2T + 2 * tau) {}
const ZZ& OLE_Interface::getTau() const {return tau;}
const ZZ& OLE_Interface::getN() const {return n;}

Vec<Vec<ZZ_p>> generateR(OLE_Interface& OLE, Poly_Field& PF, Vec<long>& alpha) {
	assert(!IsZero(ZZ_p::modulus()));
	
	// Check size of n
	ZZ n = OLE.getN();
	if (n > NTL::to_ZZ(LONG_MAX)) Error("Failed to generate R: n too large for long datatype");
	long long_n = conv<long>(n); // The hRP(alpha) in R_{n+1} will be treated separately, to avoid data type conflict 
	Vec<Vec<ZZ_p>> R;
	R.SetLength(long_n);	

	for (int i = 0; i < n-1; i++) {
		// Randomly generate polynomials
		FirstDegPolynomial tempPoly = FirstDegPolynomial(PF);
		Vec<ZZ_p> R_i;
		R_i.SetLength(2);
		R_i[0] = tempPoly.P[0];
		R_i[1] = tempPoly.P[1];
		R.append(R_i);
	}	
	return R;
}

Vec<long> generateAlpha(OLE_Interface& OLE) {
	ZZ n = OLE.getN();
	ZZ alphaTest = RandomBits_ZZ(n);
	std::cout << alphaTest << std::endl;

	if (n > NTL::to_ZZ(LONG_MAX)) Error("Failed to generate alpha: n too large for long datatype");
	long long_n = conv<long>(n); 
	Vec<long> alpha;
	alpha.SetLength(long_n);

	for (int i = 0; i < n; i++) {
		alpha[i] = RandomBnd(2);
	}
	
	return alpha;
}

Vec<ZZ_p> hRP(
		OLE_Interface& OLE, 
		FirstDegPolynomial& Poly, // P sampled by Alice 
		Vec<Vec<ZZ_p>>& R, // n randomly generated polynomials
		Vec<long>& alpha) 
{
	ZZ n = OLE.getN();
	Vec<ZZ_p> h; // temp 
	h.SetLength(2);
	h[0] = Poly.P[0];
	h[1] = Poly.P[1];
	for (long i = 0; i < n; i++) {
		if (alpha[i]) {
			h[0] -= R[i][0];
			h[1] -= R[i][1];			
		}
	}
	return h;
}


// Tasks for next time I pick this up:
// Protocol 2, step 1, start implementing (so far got the underlying interface for OLE, now use it for actual protocol)
// continue with the rest
