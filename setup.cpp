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
        using clock = std::chrono::high_resolution_clock;

        struct TimingRecord {
                std::string name;
                std::chrono::duration<double, std::micro> total_time_us{0};
        };

        TimingRecord t_total{"setupMultipleClients (total)", {}};
        TimingRecord t_avg{"Setup_C (average per client)", {}};

        auto total_start = clock::now();

        std::chrono::duration<double, std::micro> cumulative_client_time{0};

        std::vector<Setup_C> clients;
        clients.reserve(n);
        for (int i = 0; i < n; i++) {

                auto start = clock::now();

                Setup_C c = Setup_C(lambda);
                clients.push_back(c);

                auto end = clock::now();
                cumulative_client_time += end - start;
        }

        auto total_end = clock::now();

        t_total.total_time_us = total_end - total_start;

        if (n > 0) {
                t_avg.total_time_us = cumulative_client_time / n;
        }

        auto print_total = [](const TimingRecord& rec) {
                std::chrono::duration<double, std::micro> test_time = rec.total_time_us;
                std::cout << rec.name << " total execution time: \033[1;38;5;208m"
                          << test_time.count() / 1000 << "ms\033[0m" << std::endl;
        };

        print_total(t_avg);
        print_total(t_total);

        return clients;
}




