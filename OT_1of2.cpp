#include "OT_1of2.h"

void ZZ_to_mpz(mpz_t type_mpz, const NTL::ZZ_p& type_ZZp) {
	ZZ lift = rep(type_ZZp);
	std::string s conv<std::string>(lift);
	mpz_set_str(type_mpz, s.c_str(), 10);
}

mpz_t** alloc_mpz_matrix(int rows, int cols) {
    	mpz_t** M = (mpz_t**)malloc(rows * sizeof(mpz_t*));
    	for (int i = 0; i < rows; i++) {
        	M[i] = (mpz_t*)malloc(cols * sizeof(mpz_t));
        	for (int j = 0; j < cols; j++)
            		mpz_init(M[i][j]);
    	}
    	return M;
}

bigint** GenKeys(int number, bigint* &random_val_, int bit_size, int bit_size_) {
    	bigint  **random_val;
    	random_val = (mpz_t**)malloc(number * sizeof(mpz_t*));
    	random_val_ = alloc_mpz_array(number);
    	gmp_randstate_t state;
    	gmp_randinit_default(state);
    	gmp_randseed_ui(state, time(NULL));
    	for (int i = 0; i < number; ++i) {
        	random_val[i] = (mpz_t*)malloc(2 * sizeof(mpz_t));
        	mpz_init(random_val[i][0]);
        	mpz_init(random_val[i][1]);
        	mpz_urandomb(random_val[i][0], state, bit_size);
        	mpz_urandomb(random_val[i][1], state, bit_size);
        	mpz_urandomb(random_val_[i], state, bit_size_);
    	}
    	return random_val;
}

bigint* SS_v2(int size, bigint secret, bigint* random_val, bigint* &p_shares){
    	bigint *s_shares = alloc_mpz_array(size);
    	p_shares         = alloc_mpz_array(size);
    	for (int i = 0; i < size ; i++){
        	mpz_xor(s_shares[i], random_val[i], secret);
        	mpz_set(p_shares[i], random_val[i]);
    	}
    	return s_shares;
}

bigint** Enc_and_Swap(int size, bigint* share, bigint** pair, bigint **random_keys_ ){
    	bigint** result = alloc_mpz_matrix(size, 2);
    	bigint** res    = alloc_mpz_matrix(size, 2);
    	for (int i = 0; i < size ; i++){
        	mpz_xor(res[i][0], random_keys_[i][0], pair[i][0]);
        	mpz_xor(res[i][1], random_keys_[i][1], pair[i][1]);
        	if (mpz_cmp(share[i], zero)==0){
            		mpz_set(result[i][0], res[i][0]);
            		mpz_set(result[i][1], res[i][1]);
        	} else {
        		mpz_set(result[i][0], res[i][1]);
            		mpz_set(result[i][1], res[i][0]);
        	}
    	}
    	return result;
}

bigint* oblivious_filter(int size, bigint* share, bigint** pair){
    	bigint* result = alloc_mpz_array(size);
    	for (int i=0; i < size ; i++){
        	if (mpz_cmp(share[i], zero)==0){
            		mpz_set(result[i], pair[i][0]);
        	} else {
            		mpz_set(result[i], pair[i][1]);
        	}
    	}
	return result;
}

bigint* extract_result(int size, bigint* p_response, int* secret_indices, bigint**keys){
    	bigint* result = alloc_mpz_array(size);
    	for(int i = 0; i < size; i++){
        	mpz_xor(result[i], p_response[i], keys[i][secret_indices[i]]);
    	}
    	return result;
}


// Taken directly from previous implementation
bigint OT_1of2(const ZZ_p m0, const ZZ_p m1, int choice, int bits) {
	// Convert to mpz datatype first, since this implementation uses mainly ZZ_p

	int count = 1;
	int secret_bit_size = 1;

	// 1) Keys
	bigint* random_val_x;
	bigint** random_keys_ = GenKeys(count, random_val_x, key_bits, secret_bit_size);

	// 2) Receiver's choice bit s
	bigint s;
	mpz_init(s);
	mpz_set_ui(s, choice);

	// 3) Secret sharing
	bigint* p_shares;
	bigint* s_shares = SS_v2(count, s, random_val_x, p_shares);

	// 4) Sender messages
	bigint** messages_ = alloc_mpz_matrix(count, 2);
	mpz_set(messages_[0][0], m0);
	mpz_set(messages_[0][1], m1);

	// 5) Enc_and_Swap
	bigint** s_response_ = Enc_and_Swap(count, s_shares, messages_, random_keys_);

	// 6) Proxy oblivious filter
	bigint* p_perm = oblivious_filter(count, p_shares, s_response_);

	// 7) Receiver extract_result
	int secret_indices[1];
	secret_indices[0] = choice;
	bigint* final_result = extract_result(count, p_perm, secret_indices, random_keys_);
	mpz_clear(s);
	
	return final_result[0]
}
