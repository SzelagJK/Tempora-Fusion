#include "OLE.h"
#include <cassert>

using namespace NTL;

OLE_Interface::OLE_Interface(ZZ tau, ZZ log2T) : tau(std::move(tau)), n(log2T + 2 * tau) {}
const ZZ& OLE_Interface::getTau() const {return tau;}
const ZZ& OLE_Interface::getN() const {return n;}

Vec<Vec<ZZ_p>> generateR(OLE_Interface& OLE, Poly_Field& PF, Vec<long>& alpha) {
	assert(!IsZero(ZZ_p::modulus()));
	ZZ n = OLE.getN();
	Vec<Vec<ZZ_p>> R;

	for (int i = 0; i < n; i++) {
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
	// Check size n
	ZZ n = OLE.getN();
	if (n > NTL::to_ZZ(LONG_MAX)) Error("Failed to generate alpha: n too large for long datatype");
	long long_n = conv<long>(n); // could be potentially optimised if n would be a long to begin with (instead of ZZ) 
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
	long n_long = conv<long>(n);
	Vec<ZZ_p> h; // temp 
	h.SetLength(2);
	h[0] = Poly.P[0];
	h[1] = Poly.P[1];
	for (long i = 0; i < n; i++) {
		/* // Debugging
		std::cout << "COMPUTING H, ITERATION: " <<  i << std::endl;
		std::cout << "Alpha length check: " << alpha.length() << std::endl;
		std::cout << "R length check: " << R.length() << std::endl;
		std::cout << "R_i length check: " << R[i].length() << std::endl;
		*/
		if (alpha[i]) {
			h[0] -= R[i][0];
			h[1] -= R[i][1];			
		}
	}
	// Appending R and alpha to n+1
	R.append(h);
	alpha.append(1);
	return h;
}

Vec<ZZ_p> generateVector_r(OLE_Interface& OLE) {
	ZZ n = OLE.getN();
	Vec<ZZ_p> r;
	for (int i = 0; i < n + 1; i++) {
		ZZ_p rand = random_ZZ_p();
		r.append(rand);
	}
	return r;
}

Vec<Vec<ZZ_p>> PreparePairs(Vec<ZZ_p> r, Vec<Vec<ZZ_p>> R) {
	Vec<Vec<ZZ_p>> pairs;
	for (int i = 0; i < r.length(); i++) {
		Vec<ZZ_p> pair;
		pair.append(r);
		pair.append(evaluate_deg1(R[i], r[i]) + r[i]);
		pairs.append(pair);
	}
	return pairs;
}

// Tasks for next time I pick this up:
// Protocol 2, step 1, start implementing (so far got the underlying interface for OLE, now use it for actual protocol)
// continue with the rest
