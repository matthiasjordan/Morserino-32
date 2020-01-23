/*
 * MorseMode.h
 *
 *  Created on: 21.01.2020
 *      Author: mj
 */

#ifndef MORSEMODE_H_
#define MORSEMODE_H_

#include <Arduino.h>

class MorseMode
{
    public:
        MorseMode();
        virtual ~MorseMode() = default;

//        virtual void setup() = 0;
        virtual boolean menuExec(String mode) = 0;
        virtual void onPreferencesChanged() = 0;
        virtual boolean loop() = 0;
};
#endif /* MORSEMODE_H_ */

