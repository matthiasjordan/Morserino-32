#include "MorseMachine.h"


using namespace MorseMachine;



morserinoMode morseState = morseKeyer;
encoderMode encoderState = speedSettingMode;    // we start with adjusting the speed

boolean MorseMachine::isMode(morserinoMode mode) {
    return morseState == mode;
}

boolean MorseMachine::isEncoderMode(encoderMode mode) {
    return encoderState == mode;
}

