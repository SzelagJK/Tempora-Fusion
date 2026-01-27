#ifndef OLE_H
#define OLE_H

#include <NTL/ZZ.h>
#include <gmp.h>
#include <gmpxx.h>
// include relevant gmp and cryptopp

using namespace NTL;

class OLE_Interface {
	private:
		const ZZ tau, n;
	public:
		OLE_Interface(ZZ tau, ZZ log2T);
		const ZZ& getTau() const;
		const ZZ& getN() const;
		ZZ_p generateR();
};

Vec<Pair<ZZ_p,ZZ_p>> generateR(Poly_Field& PF);

// reconsider data type later
long hRP(int alpha, ZZ log2T);

#endif
