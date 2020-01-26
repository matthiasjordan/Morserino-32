#ifndef MORSEMODEKEYER_H_
#define MORSEMODEKEYER_H_

#include <Arduino.h>

// defines for keyer modi
//

#include "MorseMode.h"

class MorseModeKeyer: public MorseMode
{
    public:
//        void setup();
        boolean menuExec(String mode) override;
        void onPreferencesChanged() override;
        boolean loop() override;
        boolean togglePause() override;
};

extern MorseModeKeyer morseModeKeyer;

#endif /* MORSEMODEKEYER_H_ */
