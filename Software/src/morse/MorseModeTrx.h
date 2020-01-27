#ifndef MORSEMODETRX_H_
#define MORSEMODETRX_H_

#include <Arduino.h>
#include "MorseMode.h"

class MorseModeTrx: public MorseMode
{
    public:
        boolean menuExec(String mode) override;
        void onPreferencesChanged() override;
        boolean loop() override;
        boolean togglePause() override;
};

extern MorseModeTrx morseModeTrx;

#endif /* MORSEMODETRX_H_ */
