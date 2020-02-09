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
#include "MorseModeTennis.h"
#include "MorsePreferences.h"
#include "MorseMachine.h"
#include "MorseDisplay.h"
#include "MorseKeyer.h"
#include "MorseLoRa.h"
#include "MorseSound.h"

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
    client.printSentMessage = [](String m) {MorseDisplay::printToScroll(REGULAR, "> " + m);};
    client.challengeSound = [] (boolean ok) { ok ? MorseSound::soundSignalOK() : MorseSound::soundSignalERR(); };

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

