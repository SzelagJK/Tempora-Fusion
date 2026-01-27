#include "poly_field.h"
#include <cassert>
#include <iostream>

// m bits -> ceil of log_2(p), size T -> 2^{(d+1)*m}
// ZZ(1) << (d+1) * bits -> 1 * 2^{(d + 1) * bits} (<< NTL operator)
Poly_Field::Poly_Field(ZZ p, int d) : p(std::move(p)), d(std::move(d)), bits(NumBits(this->p)), log2T(ZZ(1) << ((d+1)*bits)) {
	std::cout << "prime p when initialising field: " << this->p << std::endl;
	ZZ_p::init(this->p); // Note: initilises field on the thread
}
const ZZ& Poly_Field::getPrime() const {return p;} 
const int Poly_Field::getDegree() const {return d;}
const long Poly_Field::getBits() const {return bits;}
const ZZ& Poly_Field::getLog2T() const {return log2T;}

FirstDegPolynomial::FirstDegPolynomial(Poly_Field& PF) {
	generateRandP(PF);
}
void FirstDegPolynomial::generateRandP(Poly_Field& PF) {
	assert(!IsZero(ZZ_p::modulus())); // ensure ZZ_p is initialised
	assert(PF.getDegree() == 1);  // ensure we are only operating on first deg polynomial
	P[0] = random_ZZ_p();
	P[1] = random_ZZ_p();
}
// Sidenote: OPE paper will define T and T^n as similar but separate sets, T refers to the polynomials as a whole
// whereas T^n refers to separate coefficients of that polynomial, so P is an element of T, and R, after expressing it 
// as (R_1, ..., R_n) is an element of T^n. Keep in mind that n is not referring to degree of polynomials held in T.
// Instead, it is the number of generated random polynomials for computational security. 
