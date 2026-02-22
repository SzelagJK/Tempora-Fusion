#include "setup.h"

// S.Setup
Setup_S::Setup_S(ZZ p, int t) : p(std::move(p)), t(std::move(t)) {
	std::cout << NumBits(this->p) << std::endl;
	assert(NumBits(this->p) >= 128);
	PF.reserve(1);
};
const int Setup_S::getLeaderQty() const {return t;};
const ZZ& Setup_S::getPrime() const {return p;};
const Poly_Field& Setup_S::getPF() const {return PF[0];};
const Vec<ZZ_p>& Setup_S::getX() const {return X;};

const void Setup_S::setFieldParams(int d) {
	std::cout << "[SETUP] Initialising Field Parameters" << std::endl;
	assert(PF.size() == 0);
	Poly_Field field = Poly_Field(p, d);
	PF.push_back(std::move(field));
};

const void Setup_S::generatePublicX() {
	std::cout << "[SETUP] Generating Public coords X" << std::endl;
	assert(!IsZero(ZZ_p::modulus()));
	assert(X.length() == 0); // Ensure X has not been already generated 
	// Detailed construction 1b, t_dash = t + 2 
	for (int i = 0; i < t + 2; i++) {
		ZZ_p rand = random_ZZ_p();
		X.append(rand);
	}
	// potentially perform checks, could be negligable however
};

// C.Setup
// Unlike S, pass a security param lambda instead of prime p, as p will be public (hence reusable, and can be hardcoded)
Setup_C::Setup_C(long lambda) {
	assert(lambda >= 2048); // Ensure security parameters is large enough, in accordance with the paper
	RSAParams params = setupRSA(lambda);
	// reduced to S.p, improve later
	sk = params.getPhi();
	pk = params.getN();
};
const ZZ& Setup_C::getSecretKey() const {return sk;};
const ZZ& Setup_C::getPublicKey() const {return pk;};

// when defining n, generate at least t+2 clients, otherwise the number of clients will be smaller than required (refer to the paper) 
std::vector<Setup_C> setupMultipleClients(long lambda, int n) {
	std::vector<Setup_C> clients;
	clients.reserve(n);
	for (int i = 0; i < n; i++) {
		Setup_C c = Setup_C(lambda);
		clients.push_back(c);
	}
	return clients;
}





