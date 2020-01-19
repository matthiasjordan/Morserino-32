#ifndef MORSEHEADCOPYING_H_
#define MORSEHEADCOPYING_H_

#include <Arduino.h>
#include "MorseMenu.h"

namespace MorseHeadCopying {

    void setup();
    boolean menuExec(String mode);
    void setupHeadCopying();
}




#endif /* MORSEHEADCOPYING_H_ */
