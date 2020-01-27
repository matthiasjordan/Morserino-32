#ifndef MORSEMODEKOCH_H
#define MORSEMODEKOCH_H

#include "MorseMode.h"

class MorseModeKoch: public MorseMode
{

    public:
        boolean menuExec(String mode) override;
        boolean loop() override;
        boolean togglePause() override;
        void onPreferencesChanged() override;

    private:
        String kochMode;
};

extern MorseModeKoch morseModeKoch;

#endif
