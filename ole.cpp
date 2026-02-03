#include "ole.h"
#include <iostream>
#include <cassert>

OLE::OLE(ZZ p, int key_bits, Poly_Field PF) : p(std::move(p)), key_bits(std::move(key_bits)), PF(std::move(PF)) {};

const ZZ_p OLE::runOLE(ZZ x_star, const Vec<ZZ>& coeff_ab) {
	// Verify if coefficients a and b have been specified, if not- continue with a random polynomial P
	FirstDegPolynomial P = FirstDegPolynomial(PF);
	if (coeff_ab.length() > 0) {
		assert(coeff_ab.length() == 2); // Ensure first degree
		std::cout << "[OLE] P coefficients specified - a: " << coeff_ab[0] << ", b: " << coeff_ab[1] << std::endl;
		P.P[0] = to_ZZ_p(coeff_ab[0]);
		P.P[1] = to_ZZ_p(coeff_ab[1]);	
	}
	int log2T = PF.getLog2T();

	ZZ tau = conv<ZZ>(key_bits); // Check how to calculate
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

	// Test
	std::cout << "[OLE] Correct evaluation: " << evaluate_deg1(P.P, input) << std::endl;
	// Step 4
	return OPE.extract_eval(obliviousResult, r);
}

// Overloaded to accept x_star as ZZ_p 
const ZZ_p OLE::runOLE(ZZ_p x_star, const Vec<ZZ>& coeff_ab) {
	FirstDegPolynomial P = FirstDegPolynomial(PF);
	if (coeff_ab.length() > 0) {
		assert(coeff_ab.length() == 2); // Ensure first degree
		std::cout << "[OLE] P coefficients specified - a: " << coeff_ab[0] << ", b: " << coeff_ab[1] << std::endl;
		P.P[0] = to_ZZ_p(coeff_ab[0]);
		P.P[1] = to_ZZ_p(coeff_ab[1]);	
	}
	int log2T = PF.getLog2T();

	ZZ tau = conv<ZZ>(key_bits); // Check how to calculate
	OPE_Interface OPE = OPE_Interface(tau, log2T); // OPE Protocol 2

	Vec<long> alpha = OPE.generateAlpha();
	Vec<Vec<ZZ_p>> R = OPE.generateR(PF, alpha);
	Vec<ZZ_p> h = OPE.hRP(P, R, alpha); // Modifies R and Alpha to n+1
	
	Vec<ZZ_p> r = OPE.generateVector_r();
	Vec<Vec<ZZ_p>> pairs = OPE.PreparePairs(r, R, x_star);

	ZZ_p obliviousResult = OPE.runOT_and_sum(pairs, alpha, key_bits);

	std::cout << "[OLE] Correct evaluation: " << evaluate_deg1(P.P, x_star) << std::endl;
	return OPE.extract_eval(obliviousResult, r);
}

// Overloaded to accept x_star and coeff_ab as ZZ_p
const ZZ_p OLE::runOLE(ZZ_p x_star, const Vec<ZZ_p>& coeff_ab) {
	FirstDegPolynomial P = FirstDegPolynomial(PF);
	if (coeff_ab.length() > 0) {
		assert(coeff_ab.length() == 2); // Ensure first degree
		std::cout << "[OLE] P coefficients specified - a: " << coeff_ab[0] << ", b: " << coeff_ab[1] << std::endl;
		P.P[0] = coeff_ab[0];
		P.P[1] = coeff_ab[1];	
	}
	int log2T = PF.getLog2T();

	ZZ tau = conv<ZZ>(key_bits); // Check how to calculate
	OPE_Interface OPE = OPE_Interface(tau, log2T); // OPE Protocol 2

	Vec<long> alpha = OPE.generateAlpha();
	Vec<Vec<ZZ_p>> R = OPE.generateR(PF, alpha);
	Vec<ZZ_p> h = OPE.hRP(P, R, alpha); // Modifies R and Alpha to n+1
	
	Vec<ZZ_p> r = OPE.generateVector_r();
	Vec<Vec<ZZ_p>> pairs = OPE.PreparePairs(r, R, x_star);

	ZZ_p obliviousResult = OPE.runOT_and_sum(pairs, alpha, key_bits);

	std::cout << "[OLE] Correct evaluation: " << evaluate_deg1(P.P, x_star) << std::endl;
	return OPE.extract_eval(obliviousResult, r);
}
