#include "coin_toss.h"

ZZ coinToss(int clientsNum, int outputLen) {
	// simulate coin tossing protocol along with the commitments for more accurate representation of runtime costs 
	// (but technically redundant for the purposes of the experiment)
	ZZ r_hat;
	RandomBits(r_hat, outputLen);
	ZZ tmp_c = commit(r_hat);
	for (int i = 0; i < clientsNum - 1; i++) {
		ZZ r_i;
		RandomBits(r_i, outputLen);
		ZZ c_i = commit(r_i);
		r_hat ^= r_i;
	}
	return r_hat; 
}

std::vector<int> determineLeaderIndices(int leaderCount, int clientCount, ZZ r_hat) {
	std::set<int> indices;
	int j = 0;
	do {
		ZZ idx = commit(conv<ZZ>(j), r_hat) % conv<ZZ>(clientCount); // conversion to avoid issues with ZZ -> int
		indices.insert(conv<int>(idx));
		j++;
	} while (indices.size() < leaderCount);
	std::vector<int> indicesVector(indices.begin(), indices.end());
	return indicesVector;
}

