#include "OLE.h"
#include <gmp.h>

OLE_Interface::OLE_Interface(ZZ tau, ZZ log2T) : tau(std::move(tau)), n(log2T + 2 * tau) {}
const ZZ& OLE_Interface::getTau() const {return tau;};
const ZZ& OLE_Interface::getN() const {return n;}

// Tasks for next time I pick this up:
// Protocol 2, step 1, start implementing (so far got the underlying interface for OLE, now use it for actual protocol)
// continue with the rest
