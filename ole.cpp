#include "ole.h"

OLE::OLE(ZZ p) : p(std::move(p)) {};

const ZZ_p OLE::runOLE(ZZ x_star) const {
	// Field and Polynomial Setup
	int key_bits = NumBits(this->p);
	Poly_Field PF = Poly_Field(this->p, 1);
	FirstDegPolynomial P = FirstDegPolynomial(PF);
	int log2T = PF.getLog2T();

	ZZ tau = conv<ZZ>(123); // Check how to calculate
	OPE_Interface OPE = OPE_Interface(tau, log2T); // OPE Protocol 2

	// Step 1 
	Vec<long> alpha = OPE.generateAlpha();
	Vec<Vec<ZZ_p>> R = OPE.generateR(PF, alpha);
	Vec<ZZ_p> h = OPE.hRP(P, R, alpha); // Modifies R and Alpha to n+1
	
	// Step 2
	Vec<ZZ_p> r = OPE.generateVector_r();
	ZZ_p input = to_ZZ_p(x_star);
	Vec<Vec<ZZ_p>> pairs = OPE.PreparePairs(r, R, input);

	// Step 3
	ZZ_p obliviousResult = OPE.runOT_and_sum(pairs, alpha, key_bits);

	// Step 4
	return OPE.extract_eval(obliviousResult, r);
}


