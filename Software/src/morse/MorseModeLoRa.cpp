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

#include "MorseDisplay.h"
#include "MorseMachine.h"
#include "MorseModeLoRa.h"
#include "MorseLoRa.h"
#include "MorseKeyer.h"
#include "MorseGenerator.h"

MorseModeLoRa morseModeLoRa;

/////////////////// Variables for LoRa: Buffer management etc

boolean MorseModeLoRa::menuExec(String mode)
{
    if (mode == "trx")
    {
        MorsePreferences::currentOptions = MorsePreferences::loraTrxOptions;               // list of available options in lora trx mode
        MorseMachine::morseState = MorseMachine::loraTrx;
        MorseDisplay::clear();
        MorseDisplay::printOnScroll(1, REGULAR, 0, "Start LoRa Trx");
        delay(600);
        MorseDisplay::clear();
        MorseDisplay::displayTopLine();
        MorseDisplay::printToScroll(REGULAR, "");      // clear the buffer

        MorseKeyer::setup();
        MorseKeyer::clearPaddleLatches();
        MorseKeyer::keyTx = false;
        MorseKeyer::onWordEnd = []()
        {
            /* finalise the string and send it to LoRA */
            MorseLoRa::cwForLora(3);
            MorseLoRa::sendWithLora();
            return false;
        };

        MorseGenerator::setStart();
        MorseGenerator::Config *genCon = MorseGenerator::getConfig();
        genCon->printChar = true;
        genCon->printCharStyle = BOLD;
        genCon->printSpaceAfterChar = false;
        genCon->timing = MorseGenerator::rx;

        MorseDisplay::getConfig()->autoFlush = true;

        MorseLoRa::receive();
    }
    return true;
}

boolean MorseModeLoRa::loop()
{
    if (MorseKeyer::doPaddleIambic())
    {
        return true;                                                        // we are busy keying and so need a very tight loop !
    }
    MorseGenerator::generateCW();
    return false;
}

boolean MorseModeLoRa::togglePause()
{
    return false;
}

void MorseModeLoRa::onPreferencesChanged()
{

}

