#include "MorseMachine.h"


using namespace MorseMachine;




morserinoMode MorseMachine::getMode() {
    return morseState;
}


boolean MorseMachine::isMode(morserinoMode mode) {
    return morseState == mode;
}

boolean MorseMachine::isEncoderMode(encoderMode mode) {
    return encoderState == mode;
}

