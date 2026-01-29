#include "poly_field.h"
#include <cassert>
#include <iostream>

// bits -> ceil of log_2(p), cardinality of T -> 2*bits, as it is F^2 
Poly_Field::Poly_Field(ZZ p, int d) : p(std::move(p)), d(std::move(d)), bits(NumBits(this->p)), log2T((2 * bits)) { 
	std::cout << "[Poly_Field] Prime p when initialising field: " << this->p << std::endl;
	std::cout << "[Poly_Field] Bits check: " << bits << std::endl;
	int CORRECT = 2 * bits;
	std::cout << "[Poly_Field] Log2T True Value: " << CORRECT << std::endl;
	std::cout << "[Poly_Field] Log2T Initialised: " << log2T << std::endl;
	ZZ_p::init(this->p); // Note: initilises field on the thread
}
const ZZ& Poly_Field::getPrime() const {return p;} 
const int Poly_Field::getDegree() const {return d;}
const long Poly_Field::getBits() const {return bits;}
const int Poly_Field::getLog2T() const {return log2T;}

FirstDegPolynomial::FirstDegPolynomial(Poly_Field& PF) {
	generateRandP(PF);
}
void FirstDegPolynomial::generateRandP(Poly_Field& PF) {
	assert(!IsZero(ZZ_p::modulus())); // ensure ZZ_p is initialised
	assert(PF.getDegree() == 1);  // ensure we are only operating on first deg polynomial
	P[0] = random_ZZ_p();
	P[1] = random_ZZ_p();
}
