#ifndef OT_H
#define OT_H

#include <gmp.h>
#include <gmpxx.h>
#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <string>
#include <cstdlib>
#include <ctime>
#include <sstream>

typedef mpz_t bigint;
extern bigint one, zero;

using namespace NTL;

// Taken directly from previous implementation of OT

mpz_t** alloc_mpz_matrix(int rows, int cols);
mpz_t* alloc_mpz_array(int n);
bigint** GenKeys(int number, bigint* &random_val_, int bit_size, int bit_size_);
bigint* SS_v2(int size, bigint secret, bigint* random_val, bigint* &p_shares);
bigint** Enc_and_Swap(int size, bigint* share, bigint** pair, bigint **random_keys_);
bigint* oblivious_filter(int size, bigint* share, bigint** pair);
bigint* extract_result(int size, bigint* p_response, int* secret_indices, bigint** keys);

void ZZp_to_mpz(mpz_t type_mpz, const ZZ_p& type_ZZp);
bigint* OT_1of2(const ZZ_p m0_, const ZZ_p m1_, int choice, int key_bits);

#endif
