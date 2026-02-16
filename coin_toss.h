#ifndef COIN_TOSS_H
#define COIN_TOSS_H

#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <vector>
#include <set>

#include "commitment.h"
#include "prf.h"

// here we willi nclude both coin toss and the deterministic fucntion G

ZZ coinToss(int clientsNum, int outputLen = 128);
std::vector<int> determineLeaderIndices(int leaderCount, int clientCount, ZZ r_hat);

#endif
