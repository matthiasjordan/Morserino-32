/******************************************************************************************************************************
 *  morse_3 Software for the Morserino-32 multi-functional Morse code machine, based on the Heltec WiFi LORA (ESP32) module ***
 *  Copyright (C) 2018  Willi Kraml, OE1WKL                                                                                 ***
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

#include <Arduino.h>

#include "MorseModeGenerator.h"
#include "MorseDisplay.h"
#include "MorseMachine.h"
#include "MorseKeyer.h"
#include "MorsePlayerFile.h"
#include "MorseModeHeadCopying.h"
#include "MorseLoRa.h"

MorseModeGenerator morseModeGenerator;

boolean MorseModeGenerator::menuExec(String mode)
{
    if (mode == "a")
    {
    }
    else if (mode == "player")
    {
        MorsePlayerFile::openAndSkip();
    }

    MorseModeGenerator::startTrainer();
    MorseModeGenerator::onPreferencesChanged();

    return true;
}

void MorseModeGenerator::startTrainer()
{
    MorseGenerator::setStart();
    MorseMachine::morseState = MorseMachine::morseGenerator;
    MorseGenerator::setup();
    MorseDisplay::clear();
    MorseDisplay::printOnScroll(0, REGULAR, 0, "Generator     ");
    MorseDisplay::printOnScroll(1, REGULAR, 0, "Start/Stop:   ");
    MorseDisplay::printOnScroll(2, REGULAR, 0, "Paddle | BLACK");
    delay(1250);
    MorseDisplay::clear();
    MorseDisplay::displayTopLine();
    MorseDisplay::clearScroll();      // clear the buffer
    active = false;
}

void MorseModeGenerator::onPreferencesChanged()
{
    MorseGenerator::handleEffectiveTrainerDisplay(MorsePreferences::prefs.trainerDisplay);
    MorseKeyer::keyTx = (MorsePreferences::prefs.keyTrainerMode == 2);

    MorseGenerator::Config *genCon = MorseGenerator::getConfig();
    genCon->onGeneratorWordEnd = []()
    {
        if (MorsePreferences::prefs.loraTrainerMode == 1)
        {
            // in generator mode and we want to send with LoRa
            MorseLoRa::cwForLora(0);
            MorseLoRa::cwForLora(3);// as we have just finished a word
            MorseLoRa::sendWithLora();// finalise the string and send it to LoRA
            delay(MorseKeyer::interCharacterSpace + MorseKeyer::ditLength);// we need a slightly longer pause otherwise the receiving end might fall too far behind...
        }
        return -1ul;
    };
    genCon->onCWElement = [](char c){
        if (MorsePreferences::prefs.loraTrainerMode == 1)
        {
            int e;
            switch (c) {
                case '0': {
                    e = 0;
                    break;
                }
                case '1': {
                    e = 1;
                    break;
                }
                default: {
                    e = 2;
                    break;
                }
            }
            MorseLoRa::cwForLora(e);
        }
    };

}

boolean MorseModeGenerator::togglePause()
{
    return false;
}

boolean MorseModeGenerator::loop()
{
    boolean activeOld = active;

    if (MorseKeyer::leftKey || MorseKeyer::rightKey)
    {                                    // touching a paddle starts and stops the generation of code
        // for debouncing:
        while (MorseKeyer::checkPaddles())
            ;                                                           // wait until paddles are released

        active = !active;

        //delay(100);
    } /// end squeeze

    ///// check stopFlag triggered by maxSequence
    if (MorseGenerator::stopFlag)
    {
        active = MorseGenerator::stopFlag = false;
    }

    if (activeOld != active)
    {
        if (!active)
        {
            MorseGenerator::keyOut(false, true, 0, 0);
            MorseDisplay::printOnStatusLine(true, 0, "Continue w/ Paddle");
        }
        else
        {
            MorseGenerator::generatorState = MorseGenerator::KEY_UP;
            MorseGenerator::genTimer = millis() - 1; // we will be at end of KEY_DOWN when called the first time, so we can fetch a new word etc...
            MorseDisplay::displayTopLine();
        }
    }

    if (active)
    {
        MorseGenerator::generateCW();
    }

    return false;
}
