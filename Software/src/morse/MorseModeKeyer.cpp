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

#include "MorseModeKeyer.h"
#include "MorseKeyer.h"
#include "MorseMachine.h"
#include "MorseGenerator.h"
#include "decoder.h"
#include "MorseDisplay.h"

MorseModeKeyer morseModeKeyer;

boolean MorseModeKeyer::menuExec(String mode)
{
    MorseKeyer::setup();

    if (mode == "a")
    {
        MorseMachine::morseState = MorseMachine::morseKeyer;
        MorseDisplay::clear();
        MorseDisplay::printOnScroll(1, REGULAR, 0, "Start CW Keyer");
        delay(500);
        MorseDisplay::getKeyerModeSymbol = MorseDisplay::getKeyerModeSymbolWOStraightKey;
        MorseDisplay::clear();
        MorseDisplay::displayTopLine();
        MorseDisplay::printToScroll(REGULAR, "");      // clear the buffer

        MorseModeKeyer::onPreferencesChanged();
    }
    return true;
}

boolean MorseModeKeyer::loop()
{
    return MorseKeyer::doPaddleIambic();
}

void MorseModeKeyer::onPreferencesChanged()
{
    unsigned char mode = MorsePreferences::prefs.keyTrainerMode;
    MorseKeyer::keyTx = (mode == 1 || mode == 2);
}

boolean MorseModeKeyer::togglePause()
{
    return false;
}
