/******************************************************************************************************************************
 *  morse_3 Software for the Morserino-32 multi-functional Morse code machine, based on the Heltec WiFi LORA (ESP32) module ***
 *  Copyright (C) 2020  Matthias Jordan, DL4MAT
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with this program.
 *  If not, see <https://www.gnu.org/licenses/>.
 *****************************************************************************************************************************/
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
        client.send("<sk>");
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
    }
    newState->onEnter();
    currentState = newState;
}

const char* TennisMachine::getState()
{
    return currentState->getName();
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
    MORSELOGLN("StateInitial entered");
    machine->client.print("Send cq or\nwait for qso!\n");

}

void TennisMachine::StateInitial::onLeave()
{
    MORSELOGLN("StateInitial left");

}

void TennisMachine::StateInitial::onMessageReceive(String message)
{
    MORSELOGLN("StateInitial received " + message);
    WordBuffer msgBuf(message);
    if (msgBuf.matches("cq de #"))
    {
        String dxCall = msgBuf.getMatch();
        machine->gameState.dx.call = dxCall;
        machine->client.printReceivedMessage(message);
        machine->switchToState(&machine->stateInviteReceived);
    }
}

void TennisMachine::StateInitial::onMessageTransmit(WordBuffer &message)
{
    String pattern = "cq de #";
    if (message.matches(pattern))
    {
        String us = message.getMatch();
        MORSELOGLN("StateInitial sent cq - off to invite sent - our call: '" + us + "'");
        String msg = message.getFullPatternMatch();
        machine->client.send(msg);
        machine->client.printSentMessage(msg);
        message.getAndClear();
        machine->gameState.us.call = us;
        machine->switchToState(&machine->stateInviteSent);
    }
    else
    {
//        machine->client.print("Send cq to continue!");
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
    MORSELOGLN("StateInviteReceived entered - invited by '" + machine->gameState.dx.call + "'");

}

void TennisMachine::StateInviteReceived::onLeave()
{
    MORSELOGLN("StateInviteReceived left");

}

void TennisMachine::StateInviteReceived::onMessageReceive(String message)
{
    MORSELOGLN("StateInviteReceived received " + message);
}

void TennisMachine::StateInviteReceived::onMessageTransmit(WordBuffer &message)
{
    String pattern = machine->gameState.dx.call + " de #";
    if (message.matches(pattern))
    {
        String us = message.getMatch();
        MORSELOGLN("ACK sent - off to answered");
        machine->client.send(message.getFullPatternMatch());
        message.getAndClear();
        machine->gameState.us.call = us;
        machine->switchToState(&machine->stateInviteAnswered);
    }
    else
    {
//        machine->client.print("Answer call to continue!\n");
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
    MORSELOGLN("StateInviteAnswered entered - dx: '" + machine->gameState.dx.call + "' us: '" + machine->gameState.us.call + "'\n");

}

void TennisMachine::StateInviteAnswered::onLeave()
{
    MORSELOGLN("StateInviteAnswered left");

}

void TennisMachine::StateInviteAnswered::onMessageReceive(String message)
{
    MORSELOGLN("StateInviteAnswered received '" + message + "'");
    String pattern = machine->gameState.us.call + " de " + machine->gameState.dx.call;
    if (message == pattern)
    {
        MORSELOGLN("Game between " + machine->gameState.dx.call + " and " + machine->gameState.us.call);
        machine->client.print("Game starts.\n");
        machine->switchToState(&machine->stateStartRoundReceiver);
    }
}

void TennisMachine::StateInviteAnswered::onMessageTransmit(WordBuffer &message)
{
    machine->client.print("Wait for DX to continue!\n");
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
    MORSELOGLN("StateInviteSent entered by '" + machine->gameState.us.call + "'");
    machine->client.print("\n");
}

void TennisMachine::StateInviteSent::onLeave()
{
    MORSELOGLN("StateInviteSent left");

}

void TennisMachine::StateInviteSent::onMessageReceive(String message)
{
    MORSELOGLN("StateInviteSent received '" + message + "'");
    String pattern = machine->gameState.us.call + " de #";
    WordBuffer msgBuf(message);
    if (msgBuf.matches(pattern.c_str()))
    {
        String dxCall = msgBuf.getMatch();
        MORSELOGLN("Received ACK from " + dxCall + " - off to state invite accepted");
        machine->client.printReceivedMessage(message);
        machine->gameState.dx.call = dxCall;
        machine->switchToState(&machine->stateInviteAccepted);
    }
}

void TennisMachine::StateInviteSent::onMessageTransmit(WordBuffer &message)
{
    machine->client.print("Wait for DX to continue!\n");
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
    MORSELOGLN("StateInviteAccepted entered dx: '" + machine->gameState.dx.call + "' us: '" + machine->gameState.us.call + "'");
}

void TennisMachine::StateInviteAccepted::onLeave()
{
    MORSELOGLN("StateInviteAccepted left");

}

void TennisMachine::StateInviteAccepted::onMessageReceive(String message)
{
    MORSELOGLN("StateInviteAccepted received " + message + "");
}

void TennisMachine::StateInviteAccepted::onMessageTransmit(WordBuffer &message)
{
    String pattern = machine->gameState.dx.call + " de " + machine->gameState.us.call;

    if (message.matches(pattern))
    {
        machine->client.printSentMessage(pattern);
        machine->client.send(pattern);
        machine->client.print("Game starts.\n");
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
    MORSELOGLN("StateStartRoundSender entered");
    machine->client.print("Give a word\ntwice to send!\n");
    firstAttempt = true;
}

void TennisMachine::StateStartRoundSender::onLeave()
{
    MORSELOGLN("StateStartRoundSender left");

}

void TennisMachine::StateStartRoundSender::onMessageReceive(String message)
{
    MORSELOGLN("StateStartRoundSender received " + message + "");
}

void TennisMachine::StateStartRoundSender::onMessageTransmit(WordBuffer &message)
{
    if (message.matches("# #"))
    {
        // Send test passed
        String challenge = message.getMatch();
        machine->gameState.challenge = challenge;
        machine->client.send(challenge);
        machine->client.printSentMessage(challenge);
        machine->client.challengeSound(true);
        message.getAndClear();
        machine->switchToState(&machine->stateWaitForAnswer);
    }
    else
    {
        if (!firstAttempt)
        {
            // Send test failed
            machine->client.challengeSound(false);
        }
    }

    firstAttempt = false;
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
    MORSELOGLN("StateWaitForAnswer entered");
}

void TennisMachine::StateWaitForAnswer::onLeave()
{
    MORSELOGLN("StateWaitForAnswer left");
}

void TennisMachine::StateWaitForAnswer::onMessageReceive(String message)
{
    MORSELOGLN("StateWaitForAnswer received " + message);
    if (message == machine->gameState.challenge)
    {
        machine->gameState.dx.points += 1;
    }
    machine->client.printScore(&machine->gameState);
    machine->switchToState(&machine->stateStartRoundReceiver);
}

void TennisMachine::StateWaitForAnswer::onMessageTransmit(WordBuffer &message)
{
    machine->client.print("Please wait for dx to proceed!");
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
    MORSELOGLN("StateStartRoundReceiver entered");

}

void TennisMachine::StateStartRoundReceiver::onLeave()
{
    MORSELOGLN("StateStartRoundReceiver left");
}

void TennisMachine::StateStartRoundReceiver::onMessageReceive(String message)
{
    MORSELOGLN("StateStartRoundReceiver received " + message);
    String pattern = "#";
    WordBuffer msgBuf(message);
    if (msgBuf.matches(pattern))
    {
        String challenge = msgBuf.getMatch();
        machine->gameState.challenge = challenge;
        machine->client.printReceivedMessage(message);
        machine->switchToState(&machine->stateChallengeReceived);
    }
}

void TennisMachine::StateStartRoundReceiver::onMessageTransmit(WordBuffer &message)
{
    machine->client.print("Wait for DX to continue!\n");
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
    MORSELOGLN("StateChallengeReceived entered");

}

void TennisMachine::StateChallengeReceived::onLeave()
{
    MORSELOGLN("StateChallengeReceived left\n");

}

void TennisMachine::StateChallengeReceived::onMessageReceive(String message)
{
    MORSELOGLN("StateChallengeReceived received " + message);
}

void TennisMachine::StateChallengeReceived::onMessageTransmit(WordBuffer &message)
{
    if (message.matches(machine->gameState.challenge))
    {
        // Challenge passed
        machine->client.print(" OK\n");
        machine->client.challengeSound(true);
        machine->gameState.us.points += 1;
    }
    else
    {
        machine->client.print(" ERR\n");
        machine->client.challengeSound(false);
    }
    machine->client.printScore(&machine->gameState);
    machine->client.send(message.getAndClear());
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
    machine->client.print("\nGame ended\n");
    machine->client.printScore(&machine->gameState);
    machine->client.print("Send <ka> to restart!\n");
}

void TennisMachine::StateEnd::onMessageReceive(String message)
{
    MORSELOGLN("StateEnd received " + message + "\n");
}

void TennisMachine::StateEnd::onMessageTransmit(WordBuffer &message)
{
    if (message.matches("<ka>"))
    {
        machine->client.print("\n");
        machine->switchToState(&machine->stateInitial);
    }
}
