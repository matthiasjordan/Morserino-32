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
#include "MorseLoRa.h"

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

    TennisMachine::Client client;
    client.print = [](String m) { MorseDisplay::printToScroll(BOLD, m);};
    client.printReceivedMessage = [](String m) { MorseDisplay::printToScroll(REGULAR, "< " + m);};
    client.send = [](String m){ morseModeTennis.send(m);};
    machine.setClient(client);

    MorseKeyer::onWordEnd = []()
    {
        morseModeTennis.sendBuffer.endWord();
        morseModeTennis.machine.onMessageTransmit(morseModeTennis.sendBuffer);
        MORSELOGLN("onWordEnd lamda");
        MorseDisplay::printToScroll(BOLD, "\n");
        return false;
    };

    MorseKeyer::onCharacter = [](String c)
    {
        MorseDisplay::printToScroll(BOLD, c);
        morseModeTennis.sendBuffer.addChar(c);
    };

    MorseDisplay::getConfig()->autoFlush = true;


    MorseLoRa::receive();

    machine.start();
    return true;
}

boolean MorseModeTennis::loop()
{
    receive();
    if (MorseKeyer::doPaddleIambic())
    {
        return true;
    }

    machine.loop();
    return false;
}

boolean MorseModeTennis::togglePause()
{
    return false;
}

void MorseModeTennis::onPreferencesChanged()
{

}


/**
 * We call this to send a message.
 */
void MorseModeTennis::send(String message)
{
    Serial.println("MMT::send " + message);
    MorseLoRa::sendWithLora((const char*) message.c_str());
}

void MorseModeTennis::receive()
{
    if (MorseLoRa::loRaBuReady())
    {
        MorseLoRa::RawPacket rp = MorseLoRa::decodePacket();
        receive(rp.payloadAsString());
    }
}

/**
 * Something calls this so we receive a message.
 */
void MorseModeTennis::receive(String message)
{
    MorseDisplay::printToScroll(REGULAR, "< " + message + "\n");
    machine.onMessageReceive(message);
}

