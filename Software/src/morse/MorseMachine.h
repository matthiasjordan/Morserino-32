#ifndef MORSEMACHINE_H_
#define MORSEMACHINE_H_

#include <Arduino.h>

namespace MorseMachine
{

    /// the states the morserino can be in - selected in top level menu
    enum morserinoMode
    {
        morseKeyer, loraTrx, morseTennis, morseTrx, morseGenerator, echoTrainer, headCopying, morseDecoder, shutDown, measureNF, invalid
    };

    // define modes for state machine of the various modi the encoder can be in
    enum encoderMode
    {
        speedSettingMode, volumeSettingMode, scrollMode
    };

    extern encoderMode encoderState;    // we start with adjusting the speed

    extern morserinoMode morseState;

    morserinoMode getMode();
    boolean isMode(morserinoMode mode);
    boolean isEncoderMode(encoderMode mode);
}

#endif /* MORSEMACHINE_H_ */
