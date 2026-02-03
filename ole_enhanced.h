#ifndef OLE_PLUS_H
#define OLE_PLUS_H

#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <NTL/vec_ZZ.h>
#include <gmp.h>

#include "ole.h"

using namespace NTL;

class OLE_enhanced {
	private:
		const ZZ p;
	public:
		OLE_enhanced(ZZ p);
		const ZZ_p runOLE_plus(ZZ x_star, const Vec<ZZ>& coeff_ab, const Vec<ZZ>& secrets_su) const;
};

// Literally the same as what you can find in OLE.h, redefined for semantic clarity
// (if one wants to use OLE without OLE_enhanced)
template<typename T>
Vec<ZZ> init_secrets_vector(T s, T u){
	Vec<ZZ> secrets;
	secrets.SetLength(2);
	secrets[0] = conv<ZZ>(s); // receiver secret
	secrets[1] = conv<ZZ>(u); // sender secret
	return secrets;
};

#endif
