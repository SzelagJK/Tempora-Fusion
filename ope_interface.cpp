#include "ope_interface.h"
#include <iostream>
#include <cassert>

using namespace NTL;

OPE_Interface::OPE_Interface(ZZ tau, int log2T) : tau(std::move(tau)), n(log2T + 2 * tau) {}
const ZZ& OPE_Interface::getTau() const {return tau;}
const ZZ& OPE_Interface::getN() const {return n;}

const Vec<Vec<ZZ_p>> OPE_Interface::generateR(Poly_Field& PF, Vec<long>& alpha) const {
	assert(!IsZero(ZZ_p::modulus()));
	ZZ n = getN();
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

const Vec<long> OPE_Interface::generateAlpha() const {
	// Check size n
	ZZ n = getN();
	if (n > NTL::to_ZZ(LONG_MAX)) Error("Failed to generate alpha: n too large for long datatype");
	long long_n = conv<long>(n); // could be potentially optimised if n would be a long to begin with (instead of ZZ) 
	Vec<long> alpha;
	std::cout << long_n << std::endl;
	alpha.SetLength(long_n);

	for (long i = 0; i < n; i++) {
		alpha[i] = RandomBnd(2);
		//alpha.append(RandomBnd(2));
	}
	return alpha;
}

const Vec<ZZ_p> OPE_Interface::hRP(
		FirstDegPolynomial& Poly, // P sampled by Alice 
		Vec<Vec<ZZ_p>>& R, // n randomly generated polynomials
		Vec<long>& alpha) const
{
	ZZ n = getN();
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

const Vec<ZZ_p> OPE_Interface::generateVector_r() const {
	ZZ n = getN();
	Vec<ZZ_p> r;
	for (int i = 0; i < n + 1; i++) {
		ZZ_p rand = random_ZZ_p();
		r.append(rand);
	}
	return r;
}

const Vec<Vec<ZZ_p>> OPE_Interface::PreparePairs(Vec<ZZ_p> r, Vec<Vec<ZZ_p>> R, ZZ_p x_star) const {
	Vec<Vec<ZZ_p>> pairs;
	for (int i = 0; i < r.length(); i++) {
		Vec<ZZ_p> pair;
		pair.append(r[i]);
		pair.append(evaluate_deg1(R[i], x_star) + r[i]);
		pairs.append(pair);
	}
	return pairs;
}

const ZZ_p OPE_Interface::runOT_and_sum(Vec<Vec<ZZ_p>>& pairs, Vec<long>& alpha, int key_bits) const {
	ZZ_p maskedOutput = conv<ZZ_p>(0);
	for (int i = 0; i < pairs.length(); i++) {
		bigint* result_ptr = OT_1of2(pairs[i][0], pairs[i][1], alpha[i], key_bits); // step 3
		char* s = mpz_get_str(NULL, 10, *result_ptr);
		ZZ z = conv<ZZ>(s);
		ZZ_p OT_choice = conv<ZZ_p>(z);
		maskedOutput += OT_choice; // step 4
		free(s);
	}
	return maskedOutput;
}

const ZZ_p OPE_Interface::extract_eval(ZZ_p& maskedOutput, Vec<ZZ_p>& r) const {
	for (int i = 0; i < r.length(); i++) {
		maskedOutput -= r[i];
	}
	return maskedOutput;
}

// Tasks for next time I pick this up:
// Protocol 2, step 1, start implementing (so far got the underlying interface for OLE, now use it for actual protocol)
// continue with the rest
