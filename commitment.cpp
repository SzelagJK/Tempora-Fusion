#include "commitment.h"

ZZ commit(const ZZ_p x, const ZZ_p r) {
	assert(!IsZero(ZZ_p::modulus()));	
	
	BLAKE3 hash;
	// using bytes instead of strong for safety
	ZZ x_rep = rep(x);
	ZZ r_rep = rep(r);
	long n_bytes[2] = {NumBytes(x_rep), NumBytes(r_rep)};

	std::vector<unsigned char> x_bytes(n_bytes[0]);
	std::vector<unsigned char> r_bytes(n_bytes[1]);
	BytesFromZZ(x_bytes.data(), x_rep, n_bytes[0]);
	BytesFromZZ(r_bytes.data(), r_rep, n_bytes[1]);

	std::vector<unsigned char> concat_bytes(n_bytes[0] + n_bytes[1]);
	std::copy(x_bytes.begin(), x_bytes.end(), concat_bytes.begin());
	std::copy(r_bytes.begin(), r_bytes.end(), concat_bytes.begin() + n_bytes[0]);

	std::string str_from_bytes(concat_bytes.begin(), concat_bytes.end());
	std::cout << "String message check: " << str_from_bytes << std::endl;

	std::string digest;
	StringSource(str_from_bytes, true,
			new HashFilter(hash,
				new HexEncoder(
					new StringSink(digest))));
	std::cout << "Hash check: " << digest << std::endl;
	//ZZ digest_ZZ(INIT_VAL, digest.c_str(), 16)
	ZZ digest_ZZ;
	ZZFromBytes(digest_ZZ, reinterpret_cast<const unsigned char*>(digest.data()), digest.size());
	std::cout << "ZZ hash check: " << digest_ZZ << std::endl;
	return digest_ZZ;
};


