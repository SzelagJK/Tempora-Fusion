#include "prf.h"

ZZ PRF_AES(const ZZ_p& x, const ZZ key, int outputSize) {
	// get block size for standard AES encryption
	const size_t BLOCK = AES::BLOCKSIZE; 
	size_t out_bytes = (outputSize + 7)/8; // used to control outputs length size
	size_t blocks = (outputSize + BLOCK - 1) / BLOCK;  
	
	ZZ enc_x = EncodeZZ_p(x);
	SecByteBlock enc_k = EncodeKeyZZ(key);

	AES::Encryption aes(enc_k, enc_k.size());
	std::vector<byte> prf_bytes;
	prf_bytes.reserve(blocks * BLOCK);

	assert(NumBytes(enc_x) <= 8);
	// ^note: AES block are processed in 16 bytes, when applying counter mode, 8 of those bytes will be decided to the counter, leaving remaining 8 to the encoding- so just remember for the input x to not exceed value of 8 bytes
	// repeat for however many aes blocks are needed, applies counter mode by default
	for (size_t i = 0; i < blocks; i++) {
		byte block[BLOCK] = {0}; 

		// ZZ input encoding byte representation 
		ZZ t = enc_x;
		for (int j = BLOCK - 1; j >= BLOCK/2; j--) {
			block[j] = conv<unsigned long>(t & 0xFF);
			t >>= 8;
		}
		 
		uint64_t ctr = i;
		for (int j = 0; j < 8; j++) {
    			block[j] = (ctr >> (8 * (7 - j))) & 0xFF;
		}


		byte out_block[BLOCK];
		aes.ProcessBlock(block, out_block);
		prf_bytes.insert(prf_bytes.end(), out_block, out_block + BLOCK);
	}

	prf_bytes.resize(out_bytes); // truncate

	ZZ result(0);
	for (size_t i = 0; i < prf_bytes.size(); i++) {
		result <<= 8;
		result += prf_bytes[i];
	}

	return result;
};

