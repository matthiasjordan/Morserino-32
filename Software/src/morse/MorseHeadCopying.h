#ifndef MORSEHEADCOPYING_H_
#define MORSEHEADCOPYING_H_

#include <Arduino.h>
#include "MorseMenu.h"

namespace MorseHeadCopying {

    enum AutoStopState
    {
        off, stop1, stop2
    };


    void setup();
    boolean menuExec(String mode);
    void setupHeadCopying();
    boolean loop();
    void onGeneratorWordEnd();

}




#endif /* MORSEHEADCOPYING_H_ */
