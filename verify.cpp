#include "verify.h"

Verify::Verify(
		int cmd, 
		ZZ_p m, 
		Vec<Vec<ZZ>> g_proof, 
		Vec<ZZ_p> puzzle_vector, 
		Vec<Vec<ZZ>> PP_eval, 
		ZZ p,
		Vec<ZZ_p> X,
		int t) : 
	cmd(std::move(cmd)),
	m(std::move(m)),
	g_proof(std::move(g_proof)),
	puzzle_vector(std::move(puzzle_vector)),
	PP_eval(std::move(PP_eval)),
	p(std::move(p)),
	X(std::move(X)),
	t(std::move(t)) {
		assert(cmd != 0);
	};

Verify::Verify(
		int cmd,
		ZZ_p m,
		ZZ o_proof,
		Vec<ZZ_p> puzzle_vector,
		Vec<ZZ> pp_u) : 
	cmd(std::move(cmd)),
	m(std::move(m)),
	o_proof(std::move(o_proof)),
	puzzle_vector(std::move(puzzle_vector)),
	pp_u(std::move(pp_u)) {
		assert(cmd == 0);
	};

void Verify::g_checkCommitments() {
	std::cout << "\n[Verify] Checking commitments' openings\n" << std::endl; 
	for (int i = 0; i < g_proof[0].length(); i++) {
		ZZ comm = commit(g_proof[0][i], g_proof[1][i]);
		bool anyValid = false;
		for (int j = 0; j < PP_eval[2].length(); j++) 
			if (comm == PP_eval[2][j]) anyValid = true;
		std::string checkOutcome = (anyValid == true) ? "\033[32mPass\033[0m" : "\033[31mFail\033[0m";
		std::cout << "Commitment check for root " << i+1 << "/" << g_proof[0].length() << ": " << checkOutcome << std::endl;
		if (anyValid != true) flag = false;
	}

};

void Verify::g_checkRoots_and_Result() {
	std::cout << "\n[Verify] Checking resulting polynomials' valid roots\n" << std::endl;
	// for every root_u with corresponding tk_u
	// standard procedure, look at SolvePuzzle for reference, or step 6 part b in detailed construction
	Vec<Vec<ZZ>> K;
	for (int i = 0; i < g_proof[1].length(); i++) {
		Vec<ZZ> ks;
		ZZ k_prime = PRF_AES(conv<ZZ_p>(1), g_proof[1][i]);
		ZZ s_prime = PRF_AES(conv<ZZ_p>(2), g_proof[1][i]);
		ks.append(k_prime);
		ks.append(s_prime);
		K.append(ks);
	}

	Vec<ZZ_p> theta;
	for (int i = 0; i < t+2; i++) {
		ZZ_p product = ZZ_p(1);
		ZZ_p sum = ZZ_p(0);
		for (int j = 0; j < K.length(); j++) {
			product *= to_ZZ_p(PRF_AES(conv<ZZ_p>(i), K[j][1]));
			sum += to_ZZ_p(PRF_AES(conv<ZZ_p>(i), K[j][0]));
		}
		ZZ_p theta_i = inv(product) * (puzzle_vector[i] - sum);
		theta.append(theta_i);
	}
	
	Vec<ZZ_p> extracted_roots = interpolate_roots(X, theta);

	for (int i = 0; i < extracted_roots.length(); i++) {
		ZZ_p val = evaluate_and_interpolate(X, theta, extracted_roots[i]);
		std::string checkOutcome = (val == 0) ? "\033[32mPass\033[0m" : "\033[31mFail\033[0m";
		std::cout << "Validity check for root " << i+1 << "/" << extracted_roots.length() << ": " << checkOutcome << std::endl;
		if (val != 0) flag = false;
	}

	Vec<ZZ_p> true_roots; // t+1 roots extracted, only t used for combination, check if matches any key held in g_proof[1] (corresponding tk_u)
	for (int i = 0; i < g_proof[0].length(); i++) {
		for (int j = 0; j < g_proof[1].length(); j++) {
			ZZ comm = commit(g_proof[0][i],  g_proof[1][i]);
			if (comm == PP_eval[2][j]) {
				true_roots.append(to_ZZ_p(g_proof[0][i]));
			}
		}
	}

	ZZ_p cons = evaluate_and_interpolate(X, theta);
	ZZ_p product = ZZ_p(1);
	for (int i = 0; i < true_roots.length(); i++) 
		product *= -true_roots[i];
	ZZ_p res_prime = cons * inv(product);

	if (res_prime != m) flag = false;
}

void Verify::g_decide() {
	std::string verificationOutcome = (flag == true) ? "\033[1;32mAccepted.\033[0m" : "\033[1;31mRejected.\033[0m";
	std::cout << "\nVerification for puzzle combination g: " << verificationOutcome << std::endl;
}

int Verify::g_verify() {
        using clock = std::chrono::high_resolution_clock;

        struct TimingRecord {
                std::string name;
                std::chrono::duration<double, std::micro> total_time_us{0};
        };

        TimingRecord t_total{"g_verify (total)", {}};
        TimingRecord t_checkCommitments{"g_checkCommitments", {}};
        TimingRecord t_checkRootsAndResult{"g_checkRoots_and_Result", {}};
        TimingRecord t_decide{"g_decide", {}};

        auto total_start = clock::now();

        {
                auto start = clock::now();
                g_checkCommitments();
                auto end = clock::now();
                t_checkCommitments.total_time_us = end - start;
        }

        {
                auto start = clock::now();
                g_checkRoots_and_Result();
                auto end = clock::now();
                t_checkRootsAndResult.total_time_us = end - start;
        }

        {
                auto start = clock::now();
                g_decide();
                auto end = clock::now();
                t_decide.total_time_us = end - start;
        }

        auto total_end = clock::now();
        t_total.total_time_us = total_end - total_start;

        auto print_total = [](const TimingRecord& rec) {
                std::chrono::duration<double, std::micro> test_time = rec.total_time_us;
                std::cout << rec.name << " total execution time: \033[1;38;5;208m"
                          << test_time.count() / 1000 << "ms\033[0m" << std::endl;
        };

        print_total(t_checkCommitments);
        print_total(t_checkRootsAndResult);
        print_total(t_decide);
        print_total(t_total);

        int out = (flag == true) ? 1 : 0;
        return out;
}

void Verify::o_checkCommitments() {
	std::cout << "\n[Verify] Checking commitment openings\n" << std::endl; 
	ZZ comm	= commit(rep(m), o_proof);
	std::string checkOutcome = (comm == pp_u[0]) ? "\033[32mPass\033[0m" : "\033[31mFail\033[0m";
	std::cout << "Commitment check: " << checkOutcome << std::endl;
}

void Verify::o_decide() {
	std::string verificationOutcome = (flag == true) ? "\033[1;32mAccepted.\033[0m" : "\033[1;31mRejected.\033[0m";
	std::cout << "\nVerification for puzzle o: " << verificationOutcome << std::endl;	
}
int Verify::o_verify() {
        using clock = std::chrono::high_resolution_clock;

        struct TimingRecord {
                std::string name;
                std::chrono::duration<double, std::micro> total_time_us{0};
        };

        TimingRecord t_total{"o_verify (total)", {}};
        TimingRecord t_checkCommitments{"o_checkCommitments", {}};
        TimingRecord t_decide{"o_decide", {}};

        auto total_start = clock::now();

        {
                auto start = clock::now();
                o_checkCommitments();
                auto end = clock::now();
                t_checkCommitments.total_time_us = end - start;
        }

        {
                auto start = clock::now();
                o_decide();
                auto end = clock::now();
                t_decide.total_time_us = end - start;
        }

        auto total_end = clock::now();
        t_total.total_time_us = total_end - total_start;

        auto print_total = [](const TimingRecord& rec) {
                std::chrono::duration<double, std::micro> test_time = rec.total_time_us;
                std::cout << rec.name << " total execution time: \033[1;38;5;208m"
                          << test_time.count() / 1000 << "ms\033[0m" << std::endl;
        };

        print_total(t_checkCommitments);
        print_total(t_decide);
        print_total(t_total);

        int out = (flag == true) ? 1 : 0;
        return out;
}


