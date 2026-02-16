#include "coin_toss.h"

ZZ coinToss(int clientsNum, int outputLen) {
	// simulate coin tossing protocol along with the commitments for more accurate representation of runtime costs 
	// (but technically redundant for the purposes of the experiment)
	ZZ r_hat = RandomBits(outputLen);
	ZZ tmp_c = commit(tmp_r);
	for (int i = 0; i < clientsNum - 1; i++) {
		ZZ r_i = RandomBits(outputLen);
		ZZ c_i = commit(r_i);
		r_hat ^= r;
	}
	return r_hat; 
}

std::set<int> determineLeaderIndices(int leaderCount, ZZ r_hat) {
	std::set<int> indices;
	int j = 0;
	do {
		ZZ idx = 
	} while (indeces.size() < leaderCount);
}

