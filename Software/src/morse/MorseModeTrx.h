#ifndef MORSEMODETRX_H_
#define MORSEMODETRX_H_

#include <Arduino.h>
#include "MorseMode.h"

class MorseModeTrx: public MorseMode
{
    public:
//        void setup();
        boolean menuExec(String mode);
        void onPreferencesChanged();
        boolean loop();
};

extern MorseModeTrx morseModeTrx;

#endif /* MORSEMODETRX_H_ */
