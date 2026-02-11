#include "prf.h"

ZZ PRF_AES(const ZZ_p& x, const ZZ_p& key, int outputSize) {
	const size_t BLOCK = AES::BLOCKSIZE; 
	size_t out_bytes = (outputSize + 7)/8;
	size_t blocks = (outputSize + BLOCK - 1) / BLOCK;  
	ZZ enc_x = EncodeZZ_p(x);
	SecByteBlock enc_k = EncodeKeyZZ_p(key);

	AES::Encryption aes(enc_k, enc_k.size());
	std::vector<byte> prf_bytes;
	prf_bytes.reserve(blocks * BLOCK);

	// repeat for however many aes blocks are needed, applies counter mode by default
	std::cout << "BLOCKS: " << blocks << std::endl;
	for (size_t i = 0; i < blocks; i++) {
		byte block[BLOCK] = {0};

		uint64_t ctr = i;
		for (int j = 0; j < 8; j++) {
    			block[j] = (ctr >> (8 * (7 - j))) & 0xFF;
		}

		
		ZZ t = enc_x;
		for (int j = BLOCK - 1; j >= 0; j--) {
			block[j] = conv<unsigned long>(t & 0xFF);
			t >>= 8;
		}

		byte out_block[BLOCK];
		aes.ProcessBlock(out_block, block);
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

// NOTES FOR TOMORROW: implement prf with arbitrary output size, refer to notes made before, try to reuse alerady implemented functions (no point of reimplementing encoding helpers, unless necessary)
// check where exactly the 2nd PRF would be used, would be useful to know what lenght will it be, refer to the other paper
// implement commitment functions, with that, hashes (shouldn't be too bad)
// test both
