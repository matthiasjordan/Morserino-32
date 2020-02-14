/*
 * decoder.h
 *
 *  Created on: 07.12.2019
 *      Author: mj
 */

#ifndef MORSEMODEDECODER_H_
#define MORSEMODEDECODER_H_

#include "MorseMode.h"

class MorseModeDecoder: public MorseMode
{
    public:

        boolean menuExec(String mode) override;
        boolean loop() override;
        boolean togglePause() override;
        void onPreferencesChanged() override;
};

extern MorseModeDecoder morseModeDecoder;

#endif /* MORSEMODEDECODER_H_ */
