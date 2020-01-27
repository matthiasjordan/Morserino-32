#ifndef MORSEMODEKEYER_H_
#define MORSEMODEKEYER_H_

#include "MorseMode.h"

class MorseModeKeyer: public MorseMode
{
    public:
        boolean menuExec(String mode) override;
        void onPreferencesChanged() override;
        boolean loop() override;
        boolean togglePause() override;
};

extern MorseModeKeyer morseModeKeyer;

#endif /* MORSEMODEKEYER_H_ */
