#include "MorseMachine.h"


using namespace MorseMachine;

encoderMode MorseMachine::encoderState = speedSettingMode;    // we start with adjusting the speed

morserinoMode MorseMachine::morseState = morseKeyer;



morserinoMode MorseMachine::getMode() {
    return morseState;
}


boolean MorseMachine::isMode(morserinoMode mode) {
    return morseState == mode;
}

boolean MorseMachine::isEncoderMode(encoderMode mode) {
    return encoderState == mode;
}

