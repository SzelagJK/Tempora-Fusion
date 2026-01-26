#include "poly_field.h"

// m bits -> ceil of log_2(p), size T -> 2^{(d+1)*m}
// ZZ(1) << (d+1) * bits -> 1 * 2^{(d + 1) * bits} (<< NTL operator)
Poly_Field::Poly_Field(ZZ p, int d) : p(std::move(p)), d(std::move(d)), bits(NumBits(p)), log2T(ZZ(1) << ((d+1)*bits)) {
	ZZ_p::init(p); // Note: initilises field on the thread
}
const ZZ& Poly_Field::getPrime() const {return p;} 
const int Poly_Field::getDegree() const {return d;}
const long Poly_Field::getBits() const {return bits;}
const ZZ& Poly_Field::getLog2T() const {return log2T;}

