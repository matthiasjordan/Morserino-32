#ifndef MORSEMODETENNIS_H_
#define MORSEMODETENNIS_H_

#include "MorseMode.h"
#include "WordBuffer.h"

class MorseModeTennis: public MorseMode
{
    public:
        boolean menuExec(String mode);
        boolean loop();
        boolean togglePause();
        void onPreferencesChanged();

    private:

        struct State
        {
                virtual void onMessageReceive(String message);
                virtual void onMessageTransmit(WordBuffer &message);
                virtual void onEnter() {};
                virtual boolean loop() {return false;};
                virtual void onLeave() {};
        };

        struct StateInitial: public State
        {
                void onMessageReceive(String message);
                void onMessageTransmit(WordBuffer &message);
                void onEnter();
                boolean loop();
                void onLeave();
        };

        struct StateEnd: public State
        {
                void onMessageReceive(String message);
                void onMessageTransmit(WordBuffer &message);
                void onEnter();
        };

        State *currentState = 0;
        StateInitial stateInitial;
        StateEnd stateEnd;

        WordBuffer sendBuffer;
        WordBuffer receiveBuffer;

        void switchToState(State *newState);
        void send(String message);
        void receive();
        void receive(String message);
};

extern MorseModeTennis morseModeTennis;

#endif /* MORSEMODETENNIS_H_ */
