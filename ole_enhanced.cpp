#include "ole_enhanced.h"
#include <iostream>

OLE_enhanced::OLE_enhanced(ZZ p) : p(std::move(p)) {};
const ZZ_p OLE_enhanced::runOLE_plus(ZZ x_star, const Vec<ZZ>& coeff_ab, const Vec<ZZ>& secrets_su) const {
        // Field and Polynomial Setup
        int key_bits = NumBits(this->p);
	Poly_Field PF = Poly_Field(this->p, 1);
	
	// Initialise base OLE
	OLE OLE_p = OLE(this->p, key_bits, PF);

	// First round OLE
	ZZ_p x_star_ZZ_p = to_ZZ_p(x_star);
	ZZ_p x_inverse = inv(x_star_ZZ_p);
	ZZ_p t = OLE_p.runOLE(x_inverse, secrets_su); // Runs through a polynomial, instead, run it through ux^-1 + s
	std::cout << "[OLE_Enhanced] Correct t: " << (to_ZZ_p(secrets_su[1])*x_inverse + to_ZZ_p(secrets_su[0])) << std::endl;
	std::cout << "[OLE_Enhanced] Computed t: " << t << std::endl;

	// Second round OLE
	Vec<ZZ_p> masked_coeff;
	masked_coeff.SetLength(2);
	// Sidenote:
	// This codebase defines polyonmials as P : a_0 + a_1x ... a_n*x^n
	// OLE+ paper defines linear functions as L : a_1x + a_0
	// As such, keep in mind to inverse indexes for coefficients when masking them in accordance with the OLE+ protocol
	masked_coeff[1] = to_ZZ_p(coeff_ab[1]) + t; // t + a
	masked_coeff[0] = to_ZZ_p(coeff_ab[0]) - to_ZZ_p(secrets_su[1]); // b - u
	ZZ_p c = OLE_p.runOLE(x_star_ZZ_p, masked_coeff);
	std::cout << "[OLE_Enhanced] Correct c: " << (masked_coeff[0] + masked_coeff[1] * x_star_ZZ_p) << std::endl;
	std::cout << "[OLE_Enhanced] Computed c: " << c << std::endl;

	// outputs correction: c = ax + b + sx -> ax + b = c - sx
	return c - (to_ZZ_p(secrets_su[0])*to_ZZ_p(x_star));
};

