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
};



#endif
