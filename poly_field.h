#ifndef FIELD_H
#define FIELD_H

#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>

using namespace NTL;

class Poly_Field {
	private:
		const ZZ p, log2T; 
		const int d;
	        const long bits; // ZZ operator<<(constZZ& x, long k), (1*2^k),  so bits and d can't be bigger than long 
	public:
		Poly_Field(ZZ p, int d);
		const ZZ& getPrime() const;
		const int getDegree() const;
		const long getBits() const;
		const ZZ& getLog2T() const;
};

class FirstDegPolynomial {
	public:
		FirstDegPolynomial(Poly_Field& PF);
		ZZ_p P[2];
		void generateRandP(Poly_Field& PF);
		// Potential sums
};

template<typename PolyContainer, typename Field>
Field evaluate_deg1(const PolyContainer& P, const Field& x_) {
	return P[0] + P[1] * x_;
};


#endif 
