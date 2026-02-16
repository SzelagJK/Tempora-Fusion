#ifndef COIN_TOSS_H
#define COIN_TOSS_H

#include <NTL/ZZ.h>
#include <NTL/ZZ_p.h>
#include <set>

#include "commit.h"
#include "prf.h"

// here we willi nclude both coin toss and the deterministic fucntion G

ZZ coinToss(int clientsNum, int outputLen);
std::set<int> G(int leaderCount, ZZ r_hat);

#endif
