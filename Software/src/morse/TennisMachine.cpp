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
    MORSELOGLN("TM::oMT " + message.get());
    if (message.matches("<sk>"))
    {
        MORSELOGLN("TM::oMT got <sk>");
        message.getAndClear();
        switchToState(&stateEnd);
    }
    else
    {
        currentState->onMessageTransmit(message);
    }
}

void TennisMachine::onMessageReceive(String message)
{
    if (message == "<sk>")
    {
        switchToState(&stateEnd);
    }
    else
    {
        currentState->onMessageReceive(message);
    }
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

TennisMachine::GameState TennisMachine::getGameState()
{
    return gameState;
}

/*****************************************************************************
 *
 *  State: INITIAL
 */
const char* TennisMachine::StateInitial::getName()
{
    return "StateInitial";
}

void TennisMachine::StateInitial::onEnter()
{
    MORSELOGLN("TM:SI:oE machine: " + String((unsigned long ) machine));
    MORSELOGLN("StateInitial entered\n");

}

void TennisMachine::StateInitial::onLeave()
{
    MORSELOGLN("StateInitial left\n");

}

void TennisMachine::StateInitial::onMessageReceive(String message)
{
    MORSELOGLN("StateInitial received " + message + "\n");
    WordBuffer msgBuf(message);
    if (msgBuf.matches("cq de #"))
    {
        String dxCall = msgBuf.getMatch();
        machine->gameState.dx.call = dxCall;
        machine->print(message);
        machine->switchToState(&machine->stateInviteReceived);
    }
}

void TennisMachine::StateInitial::onMessageTransmit(WordBuffer &message)
{
    String pattern = "cq de #";
    if (message.matches(pattern))
    {
        String us = message.getMatch();
        MORSELOGLN("StateInitial sent cq - off to invite sent - our call: '" + us + "'\n");
        machine->send(message.getFullPatternMatch());
        message.getAndClear();
        machine->gameState.us.call = us;
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
    return "StateInviteReceived";
}

void TennisMachine::StateInviteReceived::onEnter()
{
    MORSELOGLN("StateInviteReceived entered - invited by '" + machine->gameState.dx.call + "'\n");

}

void TennisMachine::StateInviteReceived::onLeave()
{
    MORSELOGLN("StateInviteReceived left\n");

}

void TennisMachine::StateInviteReceived::onMessageReceive(String message)
{
    MORSELOGLN("StateInviteReceived received " + message + "\n");
}

void TennisMachine::StateInviteReceived::onMessageTransmit(WordBuffer &message)
{
    String pattern = machine->gameState.dx.call + " de #";
    if (message.matches(pattern))
    {
        String us = message.getMatch();
        MORSELOGLN("ACK sent - off to answered\n");
        machine->send(message.getFullPatternMatch());
        message.getAndClear();
        machine->gameState.us.call = us;
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
    machine->print("StateInviteAnswered entered - dx: '" + machine->gameState.dx.call + "' us: '"+ machine->gameState.us.call+ "'\n");

}

void TennisMachine::StateInviteAnswered::onLeave()
{
    MORSELOGLN("StateInviteAnswered left\n");

}

void TennisMachine::StateInviteAnswered::onMessageReceive(String message)
{
    MORSELOGLN("StateInviteAnswered received " + message + "\n");
    String pattern = machine->gameState.dx.call + " de " + machine->gameState.us.call;
    if (message == pattern)
    {
        MORSELOGLN("The game commences between " + machine->gameState.dx.call + " and " + machine->gameState.us.call + "\n");
        machine->print(message + " GO!");
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
    MORSELOGLN("StateInviteSent entered by '" + machine->gameState.us.call + "'\n");

}

void TennisMachine::StateInviteSent::onLeave()
{
    MORSELOGLN("StateInviteSent left\n");

}

void TennisMachine::StateInviteSent::onMessageReceive(String message)
{
    machine->print("StateInviteSent received " + message + "\n");
    String pattern = machine->gameState.us.call + " de #";
    WordBuffer msgBuf(message);
    if (msgBuf.matches(pattern.c_str()))
    {
        String dxCall = msgBuf.getMatch();
        MORSELOGLN("Received ACK from " + dxCall + " - off to state invite accepted");
        machine->print(message);
        machine->gameState.dx.call = dxCall;
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
    MORSELOGLN("StateInviteAccepted entered dx: '" + machine->gameState.dx.call + "' us: '" + machine->gameState.us.call +
            "'\n");

}

void TennisMachine::StateInviteAccepted::onLeave()
{
    MORSELOGLN("StateInviteAccepted left\n");

}

void TennisMachine::StateInviteAccepted::onMessageReceive(String message)
{
    MORSELOGLN("StateInviteAccepted received " + message + "\n");
}

void TennisMachine::StateInviteAccepted::onMessageTransmit(WordBuffer &message)
{
    String pattern = machine->gameState.dx.call + " de " + machine->gameState.us.call;

    if (message.matches(pattern))
    {
        machine->print("The game commences.");
        machine->send(pattern);
        message.getAndClear();
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
    MORSELOGLN("StateStartRoundSender entered\n");
    machine->print("Give a word twice to send!\n");
}

void TennisMachine::StateStartRoundSender::onLeave()
{
    MORSELOGLN("StateStartRoundSender left\n");

}

void TennisMachine::StateStartRoundSender::onMessageReceive(String message)
{
    MORSELOGLN("StateStartRoundSender received " + message + "\n");
}

void TennisMachine::StateStartRoundSender::onMessageTransmit(WordBuffer &message)
{
    if (message.matches("# #"))
    {
        // Send test passed
        String challenge = message.getMatch();
        String pattern = machine->gameState.dx.call + " de " + machine->gameState.us.call;
        machine->gameState.challenge = challenge;
        machine->send(message.getFullPatternMatch());
        message.getAndClear();
        machine->switchToState(&machine->stateWaitForAnswer);
    }
    else
    {
        // Send test failed
        machine->print("Try again to morse a word twice!");
    }
}

/*****************************************************************************
 *
 *  State: WAIT FOR ANSWER
 */
const char* TennisMachine::StateWaitForAnswer::getName()
{
    return "StateWaitForAnswer";
}

void TennisMachine::StateWaitForAnswer::onEnter()
{
    MORSELOGLN("StateWaitForAnswer entered\n");
}

void TennisMachine::StateWaitForAnswer::onLeave()
{
    MORSELOGLN("StateWaitForAnswer left\n");
}

void TennisMachine::StateWaitForAnswer::onMessageReceive(String message)
{
    MORSELOGLN("StateWaitForAnswer received " + message + "\n");
    if (message == machine->gameState.challenge)
    {
        machine->gameState.dx.points += 1;
        machine->print("us: " + String(machine->gameState.us.points) + " dx: " + String(machine->gameState.dx.points) + "\n");
    }
    machine->switchToState(&machine->stateStartRoundReceiver);
}

void TennisMachine::StateWaitForAnswer::onMessageTransmit(WordBuffer &message)
{
    machine->print("Please wait for dx to proceed!");
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
    MORSELOGLN("StateStartRoundReceiver entered\n");

}

void TennisMachine::StateStartRoundReceiver::onLeave()
{
    MORSELOGLN("StateStartRoundReceiver left\n");
}

void TennisMachine::StateStartRoundReceiver::onMessageReceive(String message)
{
    MORSELOGLN("StateStartRoundReceiver received " + message + "\n");
    String pattern = "#";
    WordBuffer msgBuf(message);
    if (msgBuf.matches(pattern))
    {
        String challenge = msgBuf.getMatch();
        machine->gameState.challenge = challenge;
        machine->print(message);
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
    MORSELOGLN("StateChallengeReceived entered\n");

}

void TennisMachine::StateChallengeReceived::onLeave()
{
    MORSELOGLN("StateChallengeReceived left\n");

}

void TennisMachine::StateChallengeReceived::onMessageReceive(String message)
{
    MORSELOGLN("StateChallengeReceived received " + message + "\n");
}

void TennisMachine::StateChallengeReceived::onMessageTransmit(WordBuffer &message)
{
    if (message.matches(machine->gameState.challenge))
    {
        // Challenge passed
        machine->print("OK");
        machine->gameState.us.points += 1;
        machine->print("us: " + String(machine->gameState.us.points) + " dx: " + String(machine->gameState.dx.points));
    }
    else
    {
        machine->print("ERR");
    }
    machine->send(message.getAndClear());
    machine->switchToState(&machine->stateStartRoundSender);
}

/*****************************************************************************
 *
 *  State: END
 */

const char* TennisMachine::StateEnd::getName()
{
    return "StateEnd";
}

void TennisMachine::StateEnd::onEnter()
{
    machine->print("StateEnd - send <ka> to restart!\n");
}

void TennisMachine::StateEnd::onMessageReceive(String message)
{
    machine->print("StateEnd received " + message + "\n");
}

void TennisMachine::StateEnd::onMessageTransmit(WordBuffer &message)
{
    if (message.matches("<ka>"))
    {
        machine->switchToState(&machine->stateInitial);
    }
}
