/*
 * MorseModeTennis.cpp
 *
 *  Created on: 28.01.2020
 *      Author: mj
 */
#include "MorseModeTennis.h"
#include "MorsePreferences.h"
#include "MorseMachine.h"
#include "MorseDisplay.h"
#include "MorseKeyer.h"
//#include "MorseLoRa.h"

MorseModeTennis morseModeTennis;

boolean MorseModeTennis::menuExec(String mode)
{
    MorsePreferences::currentOptions = MorsePreferences::loraTrxOptions;               // list of available options in lora trx mode
    MorseMachine::morseState = MorseMachine::morseTennis;
    MorseDisplay::clear();
    MorseDisplay::printOnScroll(1, REGULAR, 0, "Start Morse Tennis");
    delay(600);
    MorseDisplay::clear();
    MorseDisplay::displayTopLine();
    MorseDisplay::printToScroll(REGULAR, "");      // clear the buffer
    MorseKeyer::setup();
    MorseKeyer::clearPaddleLatches();
    MorseKeyer::keyTx = false;

    MorseKeyer::onWordEnd = []()
    {
        morseModeTennis.currentState->onMessageTransmit(morseModeTennis.toSend);
        morseModeTennis.toSend = "";
        MorseDisplay::printToScroll(BOLD, "\n");
        return false;
    };

    MorseKeyer::onCharacter = [](String c)
    {
        MorseDisplay::printToScroll(BOLD, c);
        morseModeTennis.toSend += c;
    };

    MorseDisplay::getConfig()->autoFlush = true;

//    MorseLoRa::receive();

    switchToState(&stateInitial);

    return true;
}

boolean MorseModeTennis::loop()
{
    receive();
    return currentState->loop();
}

boolean MorseModeTennis::togglePause()
{
    return false;
}

void MorseModeTennis::onPreferencesChanged()
{

}

void MorseModeTennis::switchToState(State *newState)
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

/**
 * We call this to send a message.
 */
void MorseModeTennis::send(String message)
{
    Serial.println("SEND " + message);
}

void MorseModeTennis::receive()
{
    if (Serial.available())
    {
        String inMsg = Serial.readString();
        receive(inMsg);
    }
}

/**
 * Something calls this so we receive a message.
 */
void MorseModeTennis::receive(String message)
{
    MorseDisplay::printToScroll(REGULAR, "< " + message + "\n");
    currentState->onMessageReceive(message);
}

/*****************************************************************************
 *
 *  State: INITIAL
 */

void MorseModeTennis::StateInitial::onEnter()
{
    MorseDisplay::printToScroll(REGULAR, "Initial entered\n");

}

boolean MorseModeTennis::StateInitial::loop()
{
    if (MorseKeyer::doPaddleIambic())
    {
        return true;
    }

    return false;
}

void MorseModeTennis::StateInitial::onLeave()
{
    MorseDisplay::printToScroll(REGULAR, "Initial left\n");

}

void MorseModeTennis::StateInitial::onMessageReceive(String message)
{
    MorseDisplay::printToScroll(REGULAR, "Initial received " + message + "\n");
}

void MorseModeTennis::StateInitial::onMessageTransmit(String message)
{
    if (message == "cq")
    {
        MorseDisplay::printToScroll(REGULAR, "Initial sent cq - off to end state\n");
        morseModeTennis.send(message);
        morseModeTennis.switchToState(&morseModeTennis.stateEnd);
    }
    else
    {
        MorseDisplay::printToScroll(REGULAR, "Send cq to continue!\n");
    }
}

/*****************************************************************************
 *
 *  State: END
 */

void MorseModeTennis::StateEnd::onEnter()
{
    MorseDisplay::printToScroll(REGULAR, "End - send k on Serial to restart!\n");
}

void MorseModeTennis::StateEnd::onMessageReceive(String message)
{
    MorseDisplay::printToScroll(REGULAR, "End received " + message + "\n");
    if (message == "k")
    {
        morseModeTennis.switchToState(&morseModeTennis.stateInitial);
    }
}

void MorseModeTennis::StateEnd::onMessageTransmit(String message)
{
    MorseDisplay::printToScroll(REGULAR, "End would transmit " + message + "\n");
}
