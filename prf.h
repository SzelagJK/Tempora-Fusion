#ifndef PRF_H
#define PRF_H

#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <NTL/vec_ZZ_p.h>
#include <cryptopp/aes.h>
#include <cryptopp/hmac.h>
#include <cryptopp/sha.h>
#include <cryptopp/secblock.h>
#include <vector>
#include <iostream>
#include <cstring>

#include "helper_functions.h"

using namespace NTL;
using namespace CryptoPP;

ZZ PRF_AES(const ZZ_p& x, const ZZ_p& key, int outputSize = 128); // will initiate "counter mode" when >128 and truncate if < 128 

#endif 
