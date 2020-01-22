#ifndef MORSETRX_H_
#define MORSETRX_H_

#include <Arduino.h>
#include "MorseMode.h"

class MorseModeTrx: public MorseMode
{
    public:
        void setup();
        boolean menuExec(String mode);
        void onPreferencesChanged();
        boolean loop();
};

extern MorseModeTrx morseModeTrx;

namespace MorseTrx
{

}

#endif /* MORSETRX_H_ */
