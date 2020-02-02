/*
 * TennisMachine.cpp
 *
 *  Created on: 28.01.2020
 *      Author: mj
 */
#include "TennisMachine.h"

void TennisMachine::start()
{
    switchToState(&stateInitial);
}

void TennisMachine::loop()
{
    currentState->loop();
}

void TennisMachine::onMessageTransmit(WordBuffer &message)
{
    currentState->onMessageTransmit(message);
    MORSELOGLN("TM::oMT");
}

void TennisMachine::onMessageReceive(String message)
{
    currentState->onMessageReceive(message);
}

void TennisMachine::switchToState(State *newState)
{
    if (currentState != 0)
    {
        currentState->onLeave();
        delay(1000);
    }
    newState->onEnter();
    delay(1000);
    currentState = newState;
}

const char* TennisMachine::getState()
{
    return currentState->getName();
}

/**
 * We call this to send a message.
 */
void TennisMachine::send(String message)
{
    MORSELOGLN("TM:send() 1 ");
    client.send(message);
    MORSELOGLN("TM:send() 2");
}

void TennisMachine::print(String message)
{
    MORSELOGLN("TM:print() 1 ");
    try
    {
        client.print(message);
    }
    catch (std::exception &e)
    {
        MORSELOGLN("TM:print() 2 " + String(e.what()));
    }
    MORSELOGLN("TM:print() 2");
}

/*****************************************************************************
 *
 *  State: INITIAL
 */
const char* TennisMachine::StateInitial::getName()
{
    return "initial";
}

void TennisMachine::StateInitial::onEnter()
{
    MORSELOGLN("TM:SI:oE machine: " + String((unsigned long ) machine));
    machine->print("Initial entered\n");

}

void TennisMachine::StateInitial::onLeave()
{
    machine->print("Initial left\n");

}

void TennisMachine::StateInitial::onMessageReceive(String message)
{
    machine->print("Initial received " + message + "\n");
    if (message == "cq de B") {
        machine->print("Received cq - off to invite received");
        machine->switchToState(&machine->stateInviteReceived);
    }
}

void TennisMachine::StateInitial::onMessageTransmit(WordBuffer &message)
{
    if (message >= "cq de A")
    {
        machine->print("Initial sent cq - off to invite sent\n");
        machine->send(message.getAndClear());
        machine->switchToState(&machine->stateInviteSent);
    }
    else
    {
        machine->print("Send cq to continue!\n");
    }
}

/*****************************************************************************
 *
 *  State: INVITE RECEIVED
 */
const char* TennisMachine::StateInviteReceived::getName()
{
    return "InviteReceived";
}

void TennisMachine::StateInviteReceived::onEnter()
{
    machine->print("StateInviteReceived entered\n");

}

void TennisMachine::StateInviteReceived::onLeave()
{
    machine->print("StateInviteReceived left\n");

}

void TennisMachine::StateInviteReceived::onMessageReceive(String message)
{
    machine->print("StateInviteReceived received " + message + "\n");
}

void TennisMachine::StateInviteReceived::onMessageTransmit(WordBuffer &message)
{
    if (message >= "B de A")
    {
        machine->print("ACK sent - off to answered\n");
        machine->send(message.getAndClear());
        machine->switchToState(&machine->stateInviteAnswered);
    }
    else
    {
        machine->print("Answer call to continue!\n");
    }
}

/*****************************************************************************
 *
 *  State: INVITE ANSWERED
 */
const char* TennisMachine::StateInviteAnswered::getName()
{
    return "StateInviteAnswered";
}

void TennisMachine::StateInviteAnswered::onEnter()
{
    machine->print("StateInviteAnswered entered\n");

}

void TennisMachine::StateInviteAnswered::onLeave()
{
    machine->print("StateInviteAnswered left\n");

}

void TennisMachine::StateInviteAnswered::onMessageReceive(String message)
{
    machine->print("StateInviteAnswered received " + message + "\n");
    if (message == "A de B") {
        machine->print("The game commences\n");
        machine->switchToState(&machine->stateStartRoundReceiver);
    }
}

void TennisMachine::StateInviteAnswered::onMessageTransmit(WordBuffer &message)
{
    machine->print("Wait for DX to continue!\n");
}

/*****************************************************************************
 *
 *  State: INVITE SENT
 */
const char* TennisMachine::StateInviteSent::getName()
{
    return "StateInviteSent";
}

void TennisMachine::StateInviteSent::onEnter()
{
    machine->print("StateInviteSent entered\n");

}

void TennisMachine::StateInviteSent::onLeave()
{
    machine->print("StateInviteSent left\n");

}

void TennisMachine::StateInviteSent::onMessageReceive(String message)
{
    machine->print("StateInviteSent received " + message + "\n");
    if (message == "A de B") {
        machine->print("Received ACK - off to state invite accepted");
        machine->switchToState(&machine->stateInviteAccepted);
    }
}

void TennisMachine::StateInviteSent::onMessageTransmit(WordBuffer &message)
{
    machine->print("Wait for DX to continue!\n");
}

/*****************************************************************************
 *
 *  State: INVITE ACCEPTED
 */
const char* TennisMachine::StateInviteAccepted::getName()
{
    return "StateInviteAccepted";
}

void TennisMachine::StateInviteAccepted::onEnter()
{
    machine->print("StateInviteAccepted entered\n");

}

void TennisMachine::StateInviteAccepted::onLeave()
{
    machine->print("StateInviteAccepted left\n");

}

void TennisMachine::StateInviteAccepted::onMessageReceive(String message)
{
    machine->print("StateInviteAccepted received " + message + "\n");
}

void TennisMachine::StateInviteAccepted::onMessageTransmit(WordBuffer &message)
{
    if (message >= "B de A") {
        machine->print("The game commences.");
        machine->switchToState(&machine->stateStartRoundSender);
    }
}

/*****************************************************************************
 *
 *  State: START ROUND SENDER
 */
const char* TennisMachine::StateStartRoundSender::getName()
{
    return "StateStartRoundSender";
}

void TennisMachine::StateStartRoundSender::onEnter()
{
    machine->print("StateStartRoundSender entered\n");

}

void TennisMachine::StateStartRoundSender::onLeave()
{
    machine->print("StateStartRoundSender left\n");

}

void TennisMachine::StateStartRoundSender::onMessageReceive(String message)
{
    machine->print("StateStartRoundSender received " + message + "\n");
}

void TennisMachine::StateStartRoundSender::onMessageTransmit(WordBuffer &message)
{
    if (message == "w w") {
        // Send test passed
        machine->send("B de A w");
        machine->switchToState(&machine->stateStartRoundReceiver);
    }
    else {
        // Send test failed
        machine->print("Sorry - try again to morse 'w' twice!");
    }
}

/*****************************************************************************
 *
 *  State: START ROUND RECEIVER
 */
const char* TennisMachine::StateStartRoundReceiver::getName()
{
    return "StateStartRoundReceiver";
}

void TennisMachine::StateStartRoundReceiver::onEnter()
{
    machine->print("StateStartRoundReceiver entered\n");

}

void TennisMachine::StateStartRoundReceiver::onLeave()
{
    machine->print("StateStartRoundReceiver left\n");
}

void TennisMachine::StateStartRoundReceiver::onMessageReceive(String message)
{
    machine->print("StateStartRoundReceiver received " + message + "\n");
    if (message == "B de A w") {
        machine->switchToState(&machine->stateChallengeReceived);
    }
}

void TennisMachine::StateStartRoundReceiver::onMessageTransmit(WordBuffer &message)
{
    machine->print("Wait for DX to continue!\n");
}

/*****************************************************************************
 *
 *  State:
 */
const char* TennisMachine::StateChallengeReceived::getName()
{
    return "StateChallengeReceived";
}

void TennisMachine::StateChallengeReceived::onEnter()
{
    machine->print("StateChallengeReceived entered\n");

}

void TennisMachine::StateChallengeReceived::onLeave()
{
    machine->print("StateChallengeReceived left\n");

}

void TennisMachine::StateChallengeReceived::onMessageReceive(String message)
{
    machine->print("StateChallengeReceived received " + message + "\n");
}

void TennisMachine::StateChallengeReceived::onMessageTransmit(WordBuffer &message)
{
    if (message >= "w") {
        // Challenge passed
        machine->print("OK");
        machine->send("Bob n+1");
    }
    else {
        machine->print("ERR");
        machine->send("Bob n");
    }
    machine->switchToState(&machine->stateStartRoundSender);
}

/*****************************************************************************
 *
 *  State: END
 */

const char* TennisMachine::StateEnd::getName()
{
    return "end";
}

void TennisMachine::StateEnd::onEnter()
{
    machine->print("End - send k on Serial to restart!\n");
}

void TennisMachine::StateEnd::onMessageReceive(String message)
{
    machine->print("End received " + message + "\n");
    if (message == "k")
    {
        machine->switchToState(&machine->stateInitial);
    }
}

void TennisMachine::StateEnd::onMessageTransmit(WordBuffer &message)
{
}
