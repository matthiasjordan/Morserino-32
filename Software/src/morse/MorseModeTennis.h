#ifndef MORSEMODETENNIS_H_
#define MORSEMODETENNIS_H_

#include "MorseMode.h"
#include "WordBuffer.h"
#include "TennisMachine.h"

class MorseModeTennis: public MorseMode
{
    public:

        class ModeClient: public TennisMachine::Client
        {
                void send(String s);
                void print(String s);
                void printReceivedMessage(String s);
                void printSentMessage(String s);
                void printScore(TennisMachine::GameState *g);
                void challengeSound(boolean ok);
                void handle(TennisMachine::InitialMessageData *d);
                TennisMachine::MessageSet* getMsgSet();
        };

        boolean menuExec(String mode);
        boolean loop();
        boolean togglePause();
        void onPreferencesChanged();
        static void updateMsgSet(unsigned char msgSet, TennisMachine::GameConfig& gameConfig);
        static void updateScoringRules(unsigned char scoringRules, TennisMachine::GameConfig& gameConfig);

    private:
        WordBuffer sendBuffer;
        TennisMachine machine;
        ModeClient client;

        void receive();
        void receive(String message);
        void send(String message);
        TennisMachine::MessageSet* getMsgSet();
};

extern MorseModeTennis morseModeTennis;

#endif /* MORSEMODETENNIS_H_ */
