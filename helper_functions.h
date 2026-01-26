#ifndef HELPER_FUNCTIONS
#define HELPER_FUNCTIONS

#include <NTL/ZZ.h>
#include <string>
#include <iostream>

using namespace NTL;

NTL::ZZ messageToZZ(const std::string& m);
std::string ZZToMessage(NTL::ZZ z);

#endif

