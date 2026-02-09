#ifndef SETUP_H
#define SETUP_H

#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <NTL/vec_ZZ_p.h>
#include <vector>
#include <iostream>
#include <cassert>

#include "poly_field.h"
#include "rsa.h"

using namespace NTL;

class Setup_S {
	private:
		// Public params from S 
		const ZZ p;
		const int t; // leader clients
		Vec<ZZ_p> X; // x coords
		// Auxillery
		std::vector<Poly_Field> PF;
	public:
		Setup_S(ZZ p, int t);
		const int getLeaderQty() const;
		const ZZ& getPrime() const;
		const Poly_Field& getPF() const;
		const Vec<ZZ_p>& getX() const;
		// set, not return, easier to work with 
		const void setFieldParams(int d);
		const void generatePublicX();  					      
};

class Setup_C {
	private:
		ZZ_p sk, pk;
	public:
		Setup_C(long lambda);
		const ZZ_p& getSecretKey() const; // for testing purposes
		const ZZ_p& getPublicKey() const;
};

std::vector<Setup_C> setupMultipleClients(long lambda, int n);

#endif
