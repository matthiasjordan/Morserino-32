#ifndef TENNISMACHINE_H_
#define TENNISMACHINE_H_

#include "morsedefs.h"
#include "MorseMode.h"
#include "WordBuffer.h"

class TennisMachine
{
    public:
        static const uint8_t PROTOCOL_VERSION = 1;

        struct GameConfig {
            String cqCall;
            String dxdepat;
            String dxdeus;
            String usdedx;
            String usdepat;
            String sendChallenge;
            String answerChallenge;
        };

        struct Station {
            String call;
            uint16_t points = 0;
        };

        struct GameState {
            Station dx;
            Station us;
            String challenge;
        };

        struct InitialMessageData {
            uint8_t msgSet;
        };

        struct InitialMessageEnvelope {
            uint8_t protocolVersion = PROTOCOL_VERSION;
            InitialMessageData d;
            String text;
        };



        struct Client {
            void (*send)(String s);
            void sendAndPrint(String s) {this->send(s); this->printSentMessage(s);};
            void (*print)(String s);
            void (*printReceivedMessage)(String s);
            void (*printSentMessage)(String s);
            void (*printScore)(GameState *g);
            void (*challengeSound)(boolean ok);
        };


        void setClient(Client &c) {client = c;};
        void setGameConfig(GameConfig &c) {config = c;};

        void start();
        const char* getState();
        void loop();
        void onMessageReceive(String message);
        void onMessageTransmit(WordBuffer &message);
        GameState getGameState();

        static InitialMessageEnvelope parseInitial(String message);
        static String encodeInitial(InitialMessageEnvelope msg);


    private:

        struct State
        {
                State(TennisMachine *m) : machine{m} {};
                virtual const char* getName() = 0;
                virtual void onMessageReceive(String message) = 0;
                virtual void onMessageTransmit(WordBuffer &message) = 0;
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

        struct StateInviteReceived: public State
        {
                using State::State;
                const char* getName();
                void onMessageReceive(String message);
                void onMessageTransmit(WordBuffer &message);
                void onEnter();
                void onLeave();
        };

        struct StateInviteSent: public State
        {
                using State::State;
                const char* getName();
                void onMessageReceive(String message);
                void onMessageTransmit(WordBuffer &message);
                void onEnter();
                void onLeave();
        };

        struct StateInviteAnswered: public State
        {
                using State::State;
                const char* getName();
                void onMessageReceive(String message);
                void onMessageTransmit(WordBuffer &message);
                void onEnter();
                void onLeave();
        };

        struct StateInviteAccepted: public State
        {
                using State::State;
                const char* getName();
                void onMessageReceive(String message);
                void onMessageTransmit(WordBuffer &message);
                void onEnter();
                void onLeave();
        };

        struct StateStartRoundSender: public State
        {
                using State::State;
                const char* getName();
                void onMessageReceive(String message);
                void onMessageTransmit(WordBuffer &message);
                void onEnter();
                void onLeave();

            private:
                boolean firstAttempt;
        };

        struct StateWaitForAnswer: public State
        {
                using State::State;
                const char* getName();
                void onMessageReceive(String message);
                void onMessageTransmit(WordBuffer &message);
                void onEnter();
                void onLeave();
        };

        struct StateStartRoundReceiver: public State
        {
                using State::State;
                const char* getName();
                void onMessageReceive(String message);
                void onMessageTransmit(WordBuffer &message);
                void onEnter();
                void onLeave();
        };

        struct StateChallengeReceived: public State
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
        StateInviteReceived stateInviteReceived{this};
        StateInviteAnswered stateInviteAnswered{this};
        StateInviteSent stateInviteSent{this};
        StateInviteAccepted stateInviteAccepted{this};
        StateStartRoundSender stateStartRoundSender{this};
        StateWaitForAnswer stateWaitForAnswer{this};
        StateStartRoundReceiver stateStartRoundReceiver{this};
        StateChallengeReceived stateChallengeReceived{this};

        State *currentState = 0;

        Client client;
        GameConfig config;
        GameState gameState;

        WordBuffer sendBuffer;
        WordBuffer receiveBuffer;

        void switchToState(State *newState);
        String renderPattern(String pattern);
};

#endif /* MORSEMODETENNIS_H_ */
