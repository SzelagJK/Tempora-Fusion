#ifndef COMM_H
#define COMM_H

#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <cryptopp/blake3.h>
#include <cryptopp/hex.h>
#include <cryptopp/filters.h>
#include <vector>
#include <cstddef> // for byte data type 
#include <iostream>
#include <cassert>

using namespace NTL;
using namespace CryptoPP;

// zz as the output of the hash should be mapped as H : {0,1}* -> {0,1}^lambda
// where lambda might not be the same size as the security parameter defined in other parts of the protocol
// (hence also the params are zz);
ZZ commit(const ZZ_p x, const ZZ_p r);
ZZ commit(const ZZ x, const ZZ r);

#endif
