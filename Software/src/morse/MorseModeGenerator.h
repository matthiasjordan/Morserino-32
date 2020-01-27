#ifndef MORSEMODEGENERATOR_H_
#define MORSEMODEGENERATOR_H_

#include "MorseMode.h"

class MorseModeGenerator: public MorseMode
{
    public:
        boolean menuExec(String mode) override;
        boolean loop() override;
        void onPreferencesChanged() override;
        boolean togglePause() override;

    private:
        boolean active;
        void startTrainer();
};

extern MorseModeGenerator morseModeGenerator;

#endif /* MORSEGENERATOR_H_ */
