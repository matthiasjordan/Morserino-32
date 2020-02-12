#ifndef MORSEINPUT_H_
#define MORSEINPUT_H_

#include <Arduino.h>

namespace MorseInput {

    extern boolean useStraightKey;
//    extern void (*onCharacter)(String);
//    extern void (*onWordEnd)();

    void start(void (*onCharacter)(String), void (*onWordEnd)());
    boolean doInput();
}



#endif /* MORSEINPUT_H_ */
