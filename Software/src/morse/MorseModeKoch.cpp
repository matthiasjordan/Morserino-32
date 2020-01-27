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

#include "MorseModeKoch.h"
#include "koch.h"
#include "MorseGenerator.h"
#include "MorseModeGenerator.h"
#include "MorseModeEchoTrainer.h"

MorseModeKoch morseModeKoch;

boolean MorseModeKoch::menuExec(String mode)
{
    MorseGenerator::Config *generatorConfig = MorseGenerator::getConfig();

    kochMode = mode;

    if (mode == "learn")
    {
        morseModeEchoTrainer.startEcho();
        generatorConfig->printDitDah = true;
        generatorConfig->printChar = true;
        generatorConfig->printSpaceAfterChar = true;
        generatorConfig->clearBufferBeforPrintChar = true;
    }
    else if (mode == "trainer")
    {
        Koch::setKochActive(true);
        MorseGenerator::startTrainer();
        generatorConfig->printDitDah = false;
    }
    else if (mode == "echo")
    {
        Koch::setKochActive(true);
        morseModeEchoTrainer.startEcho();
        generatorConfig->printDitDah = false;
    }

    generatorConfig->wordEndMethod = MorseGenerator::LF;

    return true;
}

boolean MorseModeKoch::loop()
{
    if (kochMode == "learn")
    {
        return morseModeEchoTrainer.loop();
    }
    else if (kochMode == "trainer")
    {
        return morseModeGenerator.loop();
    }
    else if (kochMode == "echo")
    {
        return morseModeEchoTrainer.loop();
    }

    return false;
}

boolean MorseModeKoch::togglePause()
{
    return morseModeGenerator.togglePause();
}

void MorseModeKoch::onPreferencesChanged()
{
    return morseModeGenerator.onPreferencesChanged();
}
