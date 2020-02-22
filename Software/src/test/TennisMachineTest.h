#ifndef TENNISMACHINETEST_H_
#define TENNISMACHINETEST_H_

#include "TennisMachine.h"

void test_TennisMachine();

struct TestClient: public TennisMachine::Client
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

#endif /* TENNISMACHINETEST_H_ */
