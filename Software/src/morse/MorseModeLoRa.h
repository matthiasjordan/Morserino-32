#ifndef MORSEMODELORA_H_
#define MORSEMODELORA_H_

#include <Arduino.h>
#include "MorseMode.h"

class MorseModeLoRa: public MorseMode
{
    public:
        boolean menuExec(String mode) override;
        boolean loop() override;
        boolean togglePause() override;
        void onPreferencesChanged() override;
};

extern MorseModeLoRa morseModeLoRa;

#endif /* MORSEMODELORA_H_ */
