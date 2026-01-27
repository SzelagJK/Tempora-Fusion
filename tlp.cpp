#include "tlp.h"

TLPPuzzle::TLPPuzzle(ZZ N, ZZ r, long T, ZZ c) : N(std::move(N)), r(std::move(r)), T(std::move(T)), c(std::move(c)) {};
const ZZ& TLPPuzzle::getR() const {return r;}
const long& TLPPuzzle::getT() const {return T;}
const ZZ& TLPPuzzle::getCiphertext() const {return c;}

TLPPuzzle generatePuzzle(
    const RSAParams &params,
    const std::string &message,
    long T
) {
    ZZ r = RandomBnd(params.getN());
    ZZ a = PowerMod(ZZ(2), T, params.getPhi());
    ZZ b = PowerMod(r, a, params.getN()); // b = a^2^T 

    ZZ encodedMessage = messageToZZ(message);
    ZZ maskedMessage = encodedMessage + b % params.getN();
    std::cout << "Masked messsage: " << maskedMessage;
    TLPPuzzle o = TLPPuzzle(
		    params.getN(),
		    r,
		    T,
		    maskedMessage
		    );
    std::cout << o.getCiphertext() << std::endl;
    return o;
}

std::string solvePuzzle(const TLPPuzzle &o, RSAParams params) {
    ZZ b = o.getR();
    for (long i = 0; i < o.getT(); i++) { // note to self: sequentially compute the mask of the message to reveal it
        b = PowerMod(b, 2, params.getN());
    }

    ZZ unmaskedPuzzle = o.getCiphertext() - b;

    return ZZToMessage(unmaskedPuzzle);
}


