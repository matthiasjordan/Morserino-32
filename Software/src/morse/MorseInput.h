#ifndef MORSEINPUT_H_
#define MORSEINPUT_H_

#include <Arduino.h>

namespace MorseInput {

    void start(void (*onCharacter)(String), void (*onWordEnd)());
    boolean doInput();
    void setStraightKeyFromPrefs();
}



#endif /* MORSEINPUT_H_ */
