#ifndef MORSEMODEHEADCOPYING_H_
#define MORSEMODEHEADCOPYING_H_

#include <Arduino.h>
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
        boolean menuExec(String mode) override;
        boolean loop() override;
        void onPreferencesChanged() override;
        boolean togglePause() override;

    private:
        boolean active = false;

        unsigned long onGeneratorWordEnd();
        void startTrainer();
};

extern MorseModeHeadCopying morseModeHeadCopying;

#endif /* MORSEMODEHEADCOPYING_H_ */
