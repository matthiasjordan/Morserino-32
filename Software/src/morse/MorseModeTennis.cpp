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
#include "MorseInput.h"

MorseModeTennis morseModeTennis;

void MorseModeTennis::ModeClient::print(String m)
{
    MorseDisplay::printToScroll(BOLD, m);
}

void MorseModeTennis::ModeClient::printReceivedMessage(String m)
{
    MorseDisplay::printToScroll(FONT_INCOMING, "< " + m + "\n");
}

void MorseModeTennis::ModeClient::send(String m)
{
    morseModeTennis.send(m);
}

void MorseModeTennis::ModeClient::printSentMessage(String m)
{
    MorseDisplay::printToScroll(FONT_OUTGOING, "\n> " + m + "\n");
}

void MorseModeTennis::ModeClient::challengeSound(boolean ok)
{
    ok ? MorseSound::soundSignalOK() : MorseSound::soundSignalERR();
}

void MorseModeTennis::ModeClient::printScore(TennisMachine::GameState *g)
{
    MorseDisplay::printToScroll(BOLD, "u: " + String(g->us.points) + " dx: " + String(g->dx.points) + "\n");
}

void MorseModeTennis::ModeClient::handle(TennisMachine::InitialMessageData *d)
{
    TennisMachine::GameConfig gameConfig;
    MorseModeTennis::updateMsgSet(d->msgSet, gameConfig);
    MorseModeTennis::updateScoringRules(d->scoring, gameConfig);
    machine->setGameConfig(gameConfig);
}

TennisMachine::MessageSet *MorseModeTennis::ModeClient::getMsgSet()
{
    return &machine->getGameConfig()->msgSet;
}

void MorseModeTennis::updateMsgSet(uint8_t msgSetNo, TennisMachine::GameConfig& gameConfig)
{
    gameConfig.msgSetNo = msgSetNo;
    MORSELOGLN("MMT::updateMsgSet " + String(msgSetNo));
    switch (msgSetNo)
    {
        case 0:
        {
            gameConfig.msgSet.name = "Basic";
            gameConfig.msgSet.cqCall = "cq de #";
            gameConfig.msgSet.dxdepat = "$dx de #";
            gameConfig.msgSet.dxdeus = "$dx de $us";
            gameConfig.msgSet.usdedx = "$us de $dx";
            gameConfig.msgSet.usdepat = "$us de #";
            gameConfig.msgSet.sendChallenge = "# #";
            gameConfig.msgSet.answerChallenge = "#";
            break;
        }
        case 1:
        {
            gameConfig.msgSet.name = "Advanced";
            gameConfig.msgSet.cqCall = "cq cq cq de # # # +";
            gameConfig.msgSet.dxdepat = "$dx de # k";
            gameConfig.msgSet.dxdeus = "$dx de $us k";
            gameConfig.msgSet.usdedx = "$us de $dx k";
            gameConfig.msgSet.usdepat = "$us de # k";
            gameConfig.msgSet.sendChallenge = "# #";
            gameConfig.msgSet.answerChallenge = "#";
            break;
        }
        case 2:
        {
            gameConfig.msgSet.name = "Top notch";
            gameConfig.msgSet.cqCall = "cq cq cq de # # # +";
            gameConfig.msgSet.dxdepat = "$dx de # k";
            gameConfig.msgSet.dxdeus = "$dx de $us k";
            gameConfig.msgSet.usdedx = "$us de $dx k";
            gameConfig.msgSet.usdepat = "$us de # k";
            gameConfig.msgSet.sendChallenge = "$dx de $us # # k";
            gameConfig.msgSet.answerChallenge = "$dx de $us # k";
            break;
        }
    }
}

void MorseModeTennis::updateScoringRules(unsigned char scoringRules, TennisMachine::GameConfig& gameConfig)
{
    gameConfig.scoringNo = scoringRules;
    switch (scoringRules)
    {
        case 0:
        {
            gameConfig.senderPoints = 1;
            gameConfig.receiverPoints = 1;
            break;
        }
        case 1:
        {
            gameConfig.senderPoints = 0;
            gameConfig.receiverPoints = 1;
            break;
        }
    }
}

boolean MorseModeTennis::menuExec(String mode)
{
    MorsePreferences::currentOptions = MorsePreferences::morseTennisOptions;               // list of available options in lora trx mode
    MorseMachine::morseState = MorseMachine::morseTennis;
    MorseDisplay::getKeyerModeSymbol = MorseDisplay::getKeyerModeSymbolWStraightKey;
    MorseDisplay::clear();
    MorseDisplay::printOnScroll(0, REGULAR, 4, "Start");
    MorseDisplay::printOnScroll(1, REGULAR, 1, "Morse Tennis");
    delay(600);
    MorseDisplay::clear();
    MorseDisplay::displayTopLine();
    MorseDisplay::clearScroll();
    MorseKeyer::setup();
    MorseKeyer::clearPaddleLatches();
    MorseKeyer::keyTx = false;

    unsigned char msgSet = MorsePreferences::prefs.tennisMsgSet;
    unsigned char scoringRules = MorsePreferences::prefs.tennisScoringRules;
    TennisMachine::GameConfig gameConfig;
    updateMsgSet(msgSet, gameConfig);
    updateScoringRules(scoringRules, gameConfig);
    machine.setGameConfig(gameConfig);
    machine.setClient(&client);

    MorseInput::start([](String c)
    {
        MORSELOGLN("char " + c);
        MorseDisplay::printToScroll(FONT_OUTGOING, c);
        morseModeTennis.sendBuffer.addChar(c);
    }, []()
    {
        MorseDisplay::printToScroll(FONT_OUTGOING, " ");
        morseModeTennis.sendBuffer.endWord();
        morseModeTennis.machine.onMessageTransmit(morseModeTennis.sendBuffer);
    });

    MorseDisplay::getConfig()->autoFlush = true;

    onPreferencesChanged();

    MorseLoRa::receive();

    machine.start();
    return true;
}

boolean MorseModeTennis::loop()
{
    receive();
    if (MorseInput::doInput())
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
    MorseInput::setStraightKeyFromPrefs();
}

/**
 * We call this to send a message.
 */
void MorseModeTennis::send(String message)
{
    MORSELOGLN("MMT::send " + message);
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
    machine.onMessageReceive(message);
}

