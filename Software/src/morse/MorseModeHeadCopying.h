#ifndef MORSEMODEHEADCOPYING_H_
#define MORSEMODEHEADCOPYING_H_

#include <Arduino.h>
#include "MorseMenu.h"
#include "MorseMode.h"
#include "MorseGenerator.h"

class MorseModeHeadCopying: public MorseMode
{
    public:

        enum AutoStopState
        {
            off, stop1, stop2
        };

        const MorseGenerator::WordEndMethod wordEndMethod = MorseGenerator::LF;

        void setup();
        boolean menuExec(String mode);
        boolean loop();
        void onPreferencesChanged();
        boolean togglePause();

    private:
        boolean active = false;

        void setupHeadCopying();
        unsigned long onGeneratorWordEnd();
        void startTrainer();
};

extern MorseModeHeadCopying morseModeHeadCopying;

#endif /* MORSEMODEHEADCOPYING_H_ */
