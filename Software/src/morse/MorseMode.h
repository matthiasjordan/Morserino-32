/*
 * MorseMode.h
 *
 *  Created on: 21.01.2020
 *      Author: mj
 */

#ifndef MORSEMODE_H_
#define MORSEMODE_H_

#include "arduino.h"

class MorseMode
{
    public:
        MorseMode();
        virtual ~MorseMode() = default;

        virtual boolean menuExec(String mode) = 0;
        virtual boolean loop() = 0;

        /**
         * returns false if the mode is paused now.
         */
        virtual boolean togglePause() = 0;

        /**
         * Called when the user changed the preferences.
         */
        virtual void onPreferencesChanged() = 0;

};

#endif /* MORSEMODE_H_ */

