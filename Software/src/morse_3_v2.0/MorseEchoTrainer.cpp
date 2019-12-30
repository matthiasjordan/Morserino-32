#include "MorseEchoTrainer.h"

using namespace MorseEchoTrainer;

echoStates echoTrainerState = START_ECHO;


boolean MorseEchoTrainer::isState(echoStates state) {
    return echoTrainerState == state;
}

void MorseEchoTrainer::setState(echoStates newState) {
    echoTrainerState = newState;
}

MorseEchoTrainer::echoStates getState() {
    return echoTrainerState;
}


void MorseEchoTrainer::storeCharInResponse(String symbol) {
    symbol.replace("<as>", "S");
    symbol.replace("<ka>", "A");
    symbol.replace("<kn>", "N");
    symbol.replace("<sk>", "K");
    symbol.replace("<ve>", "V");
    symbol.replace("<ch>", "H");
    echoResponse.concat(symbol);
}
