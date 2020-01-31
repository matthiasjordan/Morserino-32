#ifndef MORSEMODETENNIS_H_
#define MORSEMODETENNIS_H_

#include "MorseMode.h"
#include "WordBuffer.h"
#include "TennisMachine.h"

class MorseModeTennis: public MorseMode
{
    public:
        boolean menuExec(String mode);
        boolean loop();
        boolean togglePause();
        void onPreferencesChanged();

    private:
        WordBuffer sendBuffer;
        TennisMachine machine;

        void receive();
        void receive(String message);
        void send(String message);
};

extern MorseModeTennis morseModeTennis;

#endif /* MORSEMODETENNIS_H_ */
