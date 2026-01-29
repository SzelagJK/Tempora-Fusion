#ifndef OLE_H
#define OLE_H

#include <NTL/ZZ.h>
#include <gmp.h>
#include <NTL/ZZ_p.h>

#include "ope_interface.h"

using namespace NTL;

class OLE {
	private:
		const ZZ p;
	public:
		OLE(ZZ p);
		const ZZ_p runOLE(ZZ x_star) const;
};

#endif
