#ifndef HELPER_FUNCTIONS
#define HELPER_FUNCTIONS

#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <cryptopp/secblock.h>
#include <string>
#include <iostream>
#include <cassert>

using namespace NTL;
using namespace CryptoPP;

ZZ messageToZZ(const std::string& m);
std::string ZZToMessage(ZZ z);
size_t FieldByteLength();
ZZ EncodeZZ_p(const ZZ_p& zz);
SecByteBlock EncodeKeyZZ_p(const ZZ_p& key);

#endif

