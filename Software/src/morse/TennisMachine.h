#ifndef TENNISMACHINE_H_
#define TENNISMACHINE_H_

#include "morsedefs.h"
#include "MorseMode.h"
#include "WordBuffer.h"

class TennisMachine
{
    public:
        struct Client {
            void (*send)(String s);
            void (*print)(FONT_ATTRIB a, String s);
        };

        void setClient(Client &c) {client = c;};

        void start();
        const char* getState();
        void loop();
        void onMessageReceive(String message);
        void onMessageTransmit(WordBuffer &message);

    private:

        struct State
        {
                State(TennisMachine *m) : machine{m} {};
                virtual const char* getName();
                virtual void onMessageReceive(String message);
                virtual void onMessageTransmit(WordBuffer &message);
                virtual void onEnter() {};
                virtual boolean loop() {return false;};
                virtual void onLeave() {};

                TennisMachine *machine;
        };

        struct StateInitial: public State
        {
                using State::State;
                const char* getName();
                void onMessageReceive(String message);
                void onMessageTransmit(WordBuffer &message);
                void onEnter();
                void onLeave();
        };

        struct StateEnd: public State
        {
                using State::State;
                const char* getName();
                void onMessageReceive(String message);
                void onMessageTransmit(WordBuffer &message);
                void onEnter();
        };

        StateInitial stateInitial{this};
        StateEnd stateEnd{this};

        State *currentState = 0;

        Client client;

        WordBuffer sendBuffer;
        WordBuffer receiveBuffer;

        void switchToState(State *newState);
        void send(String message);
        void print(FONT_ATTRIB a , String message);
};

#endif /* MORSEMODETENNIS_H_ */
